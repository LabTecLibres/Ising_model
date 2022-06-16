// LIBS
#include <windows.h>
#include <stdlib.h>
#include <gtk/gtk.h> 	        // GUI, Gtk Lib
#include "mt64.h"				// Pseudo Random Number Generation 64bit MT Lib	
#include <math.h>				// math to transform random n from continuos to discrete
#include <time.h>				// time library to initialize random generator with time seed
#include <stdio.h>

// MACROS						// to later defince a 2D Latice of 250 x 250 sites
#define X_SIZE 250				// Define una constante
#define Y_SIZE 250

// STRUCTURE with the simulation DATA
struct simulation					//Permite combinar datos de diferente tipo
	{
	int lattice_configuration[X_SIZE][Y_SIZE];      // Latice configuration
	gint run; 			        	// time handler tag. Standard C unsigned int type. Values can range from 0 to G_MAXUINT
	gboolean running;			    	// Are we running? A standard boolean type, should only contain the value TRUE or FALSE
	gboolean initialized;				// Have we been initialized?
	int generation_time;				// generations simulated
	double occupancy;			    	// lattice occupancy
	int K;
	//float T;			    		// acople
	//float J;			    			// J
	} s;						// instance s of the structure to hold the simulation

int J = -1;
float T;
double birth_rate = 1.00;

// Declare PUT PIXEL function to access individual pixel data on a Pixel Buffer. Implemented at the end of document.
void put_pixel(GdkPixbuf *pixbuf, int x, int y, guchar red, guchar green, guchar blue, guchar alpha);	
//guchar: Corresponds to the standard C unsigned char type.

// Pain a background function to make a pixel buffern and an image to display as default canvas
// Función que pinta el fondo antes de iniciar
static void paint_a_background (gpointer data)
	{
	GdkPixbuf *p;
	p = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8, X_SIZE, Y_SIZE);	
	/*Paint a background canvas for start up image*/
  	int x,y;
	for (x = 0; x < X_SIZE; x++)
    	{
        for (y = 0; y < Y_SIZE; y++)
            {
            put_pixel(p, (int)x, (int)y, (guchar)x, (guchar)y, (guchar)x, 255);
            }
        }
	gtk_image_set_from_pixbuf(GTK_IMAGE(data), GDK_PIXBUF(p));
	g_object_unref(p);
	}
	
//Función que cuenta las partículas en estado 0, 1, 2, 3 e imprime los parámetros de la simulación
static void count (gpointer data)
	{
	GdkPixbuf *p;
	p = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8, X_SIZE, Y_SIZE);
	int x, y;
	int RP = 0;
	int GP = 0;
	int BP = 0;
	int WP = 0;
	for (x = 0; x < X_SIZE; x++)
		{
		for (y = 0; y < Y_SIZE; y++)
			{
			switch(s.lattice_configuration[x][y])
				{
				case 0:
					WP++;
					put_pixel(p, (int)x, (int)y, (guchar)255, (guchar)255, (guchar)255, 255);
					break;
				case 1:
					BP++;
					put_pixel(p, (int)x, (int)y, (guchar)0, (guchar)0, (guchar)0, 255);
					break;
				case 2:
					RP++;
					put_pixel(p, (int)x, (int)y, (guchar)218, (guchar)0, (guchar)0, 255);
					break;
				case 3:
					GP++;
					put_pixel(p, (int)x, (int)y, (guchar)0, (guchar)201, (guchar)29, 255);
					break;
				default:
					put_pixel(p, (int)x, (int)y, (guchar)0, (guchar)0, (guchar)0, 255);
					break;
				}
			}
		}
	FILE *fp;
	fp = fopen("Ising.txt", "a+");		//Crea/Abre el archivo y escribe en él
	//fprintf(fp, "Gen:\tOccupancy:\tCoup:\tJ:\tWithe:\tBlack:\tRed:\tGreen:\tK:\tW+B+R+G:\n");
	fprintf(fp, "%u\t%f\t%.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", s.generation_time, (s.occupancy/(X_SIZE * Y_SIZE)), T, J, WP, BP, RP, GP, s.K, (WP+BP+RP+GP)); 			//Imprime en el archivo
	fclose(fp);
    if(p != NULL) 
        {
        gchar *str = g_strdup_printf ("T_%.2f_J_%d_birth_rate_%.2f_%d.png", T, J, birth_rate, s.K);
        cairo_surface_t *surf = cairo_image_surface_create(CAIRO_FORMAT_RGB24, X_SIZE, Y_SIZE);
        cairo_t *cr = cairo_create(surf);
        gdk_cairo_set_source_pixbuf(cr, p, 0, 0);
        cairo_paint(cr);
        cairo_surface_write_to_png(surf, str);
        g_print("Screenshot saved as T_%.2f_J_%d_%d.png\n", T, J, s.K);
        }
    else 
        {
        g_print("Unable to get the screenshot.\n");
        }
	}

// Function to paint lattice DATA from the simulation into a pixbuffer
// Función que pinta las partículas
static void paint_lattice (gpointer data)
	{
   	GdkPixbuf *p;
	p = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8, X_SIZE, Y_SIZE);
	// Paint the lattice configuration to a pixbuffer
	int x,y;
	for (x = 0; x < X_SIZE; x++)
    	{
		for (y = 0; y < Y_SIZE; y++)
			{
			switch(s.lattice_configuration[x][y])
				{
				case 0:	//Vacío: Blanco
					put_pixel(p, (int)x, (int)y, (guchar)255, (guchar)255, (guchar)255, 255);
					break;
				case 1:	//Ocupado: Negro
					put_pixel(p, (int)x, (int)y, (guchar)0, (guchar)0, (guchar)0, 255);
					break;
				case 2:	//+1: Rojo
					put_pixel(p, (int)x, (int)y, (guchar)218, (guchar)0, (guchar)0, 255);
					break;
				case 3:	//-1: Verde
					put_pixel(p, (int)x, (int)y, (guchar)0, (guchar)201, (guchar)29, 255);
					break;
				default:
					put_pixel(p, (int)x, (int)y, (guchar)0, (guchar)0, (guchar)0, 255);
					break;
				}
			}
        }
	gtk_image_set_from_pixbuf(GTK_IMAGE(data), GDK_PIXBUF(p));
	g_object_unref(p);
	}

//Función que calcula la energía de un punto
int energy (int random_x_coor2, int random_y_coor2)
	{
	//int j = s.J;
	int E0, Et;
	int R = 0;
	int G = 0;
    if      (s.lattice_configuration[random_x_coor2][random_y_coor2] == 2){E0 = 1;}
	else if (s.lattice_configuration[random_x_coor2][random_y_coor2] == 3){E0 =-1;}
	
    if      (s.lattice_configuration[random_x_coor2][(int)((Y_SIZE + random_y_coor2+1)%Y_SIZE)] == 2){R++;}
	else if (s.lattice_configuration[random_x_coor2][(int)((Y_SIZE + random_y_coor2+1)%Y_SIZE)] == 3){G++;}
	
    if      (s.lattice_configuration[random_x_coor2][(int)((Y_SIZE + random_y_coor2-1)%Y_SIZE)] == 2){R++;}
	else if (s.lattice_configuration[random_x_coor2][(int)((Y_SIZE + random_y_coor2-1)%Y_SIZE)] == 3){G++;}
	
    if      (s.lattice_configuration[(int)((X_SIZE + random_x_coor2-1)%X_SIZE)][random_y_coor2] == 2){R++;}
	else if (s.lattice_configuration[(int)((X_SIZE + random_x_coor2-1)%X_SIZE)][random_y_coor2] == 3){G++;}
	
    if      (s.lattice_configuration[(int)((X_SIZE + random_x_coor2+1)%X_SIZE)][random_y_coor2] == 2){R++;}
	else if (s.lattice_configuration[(int)((X_SIZE + random_x_coor2+1)%X_SIZE)][random_y_coor2] == 3){G++;}
	
	Et = J*E0*(R-G);
	return Et;
	}

// Up Date Function which simulates the stochastic process and updates the lattice configuration
int update_lattice(gpointer data)
    {
	long random_number; 
	double random_number1, random_number2;
	int random_neighbor_state;
	long random_neighboor;
	int x, y, E1, E2, dE1, dE2;
	//double T = s.T;
	double B1, B2;
	int sites = 0;
	int random_x_coor, random_y_coor, random_x_coor2, random_y_coor2;
	double probabily_of_state = 0.1;
	double death_rate = 0.0001;
	for (sites; sites < (int)(Y_SIZE*X_SIZE); sites++)
		{
		//pick a random focal site
		random_x_coor = (int) floor(genrand64_real3()*X_SIZE); 
		random_y_coor = (int) floor(genrand64_real3()*Y_SIZE);
		switch(s.lattice_configuration[random_x_coor][random_y_coor])
			{
			case 0: // if the site is empty
				// chose a random neighboor from the 4 posible ones
				random_neighboor = (long) floor(genrand64_real3()*4);
				switch(random_neighboor)
					{
					case 0: // south
							random_neighbor_state = s.lattice_configuration[random_x_coor][(int)((Y_SIZE + random_y_coor-1)%Y_SIZE)]; 
							break;
					case 1: // north
							random_neighbor_state =	s.lattice_configuration[random_x_coor][(int)((Y_SIZE + random_y_coor+1)%Y_SIZE)]; 
							break;
					case 2: // east
							random_neighbor_state =	s.lattice_configuration[(int)((X_SIZE + random_x_coor-1)%X_SIZE)][random_y_coor];
							break;
					case 3: // west
							random_neighbor_state =	s.lattice_configuration[(int)((X_SIZE + random_x_coor+1)%X_SIZE)][random_y_coor];
							break;
					}
				// if its random neighbor is occupied: put a 1 copy at the focal site. 
				// Note that when all 4 newighboors are occupied colonization of the focal site happens with probability 1.0.
				if (genrand64_real2() < birth_rate)
    				{
    				if(random_neighbor_state == 1)
        				{
        				s.lattice_configuration[random_x_coor][random_y_coor]= 1; s.occupancy ++;
        				}
        			else if(random_neighbor_state == 2)
        				{
        				s.lattice_configuration[random_x_coor][random_y_coor]= 2; s.occupancy ++;
        				}
        			else if(random_neighbor_state == 3)
        				{
        				s.lattice_configuration[random_x_coor][random_y_coor]= 3; s.occupancy ++;
        				}
                    }
				break; 
			case 1: // if the site is occupied, células adoptan estado +1 o -1 con cierta probabilidad
				// if a particle is present at the focal site, it can die with probability dead_rate* dt
				if (genrand64_real2() < death_rate)
    				{
    				s.lattice_configuration[random_x_coor][random_y_coor]= 0; s.occupancy --;
    				}
				else
    				{
    				if (genrand64_real1() < probabily_of_state)
        				{
        				random_number = (long) floor(genrand64_real3()*2);
        				if (random_number == 0)
                			{
            				s.lattice_configuration[random_x_coor][random_y_coor]= 2;
                        	}
            			else if (random_number == 1)
                			{
            				s.lattice_configuration[random_x_coor][random_y_coor]= 3;
            				}
        				}
        			}	
    			
				break;
            }
        int MC = 0;
        for (MC; MC < 100; MC++)
    		{
      		//pick a second random focal site
      		random_x_coor2 = (int) floor(genrand64_real3()*X_SIZE); 
      		random_y_coor2 = (int) floor(genrand64_real3()*Y_SIZE);
      		switch(s.lattice_configuration[random_x_coor2][random_y_coor2])
      			{
      			case 2: // Calcula la energía de un punto
          			if (genrand64_real2() < death_rate)
        				{
        				s.lattice_configuration[random_x_coor2][random_y_coor2]= 0; s.occupancy --;
        				}
    				else
        				{
          				random_number1 = genrand64_real3(); // r must be a random number such that 0 < r < 1
          				E1 = energy (random_x_coor2, random_y_coor2);
          				dE1 = -E1-E1;
          				B1 = exp(-dE1/T);
          				if (dE1 < 0 || random_number1 < B1)
              				{
              				s.lattice_configuration[random_x_coor2][random_y_coor2] = 3;
              				}
              			}	
      				break;
      			case 3: //
          			if (genrand64_real2() < death_rate)
        				{
        				s.lattice_configuration[random_x_coor2][random_y_coor2]= 0; s.occupancy --;
        				}
        			else
            			{	
          				random_number2 = genrand64_real3(); // r must be a random number such that 0 < r < 1
          				E2 = energy (random_x_coor2, random_y_coor2);
          				dE2 = -E2-E2;
          				B2 = exp(-dE2/T);
                          if (dE2 < 0 || random_number2 < B2)
              				{
              				s.lattice_configuration[random_x_coor2][random_y_coor2] = 2;
              				}
              			}		
      				break;
      			}
      		}
  		}
	s.generation_time ++;
	//paint_lattice (data);
	g_print ("\tGen:\t%u\n", s.generation_time);
	if(s.generation_time == 200) 			//Si el tiempo generacional alcanza un cierto número, la simulación se detiene
		{
		g_source_remove(s.run);
		s.running = FALSE;
		g_print ("Simulation Stopped\n");
        count (data); 				//Llama a la función que cuenta las partículas
		//exit (0);				//Termina la simulación
		}
	return 0;
	}

// TIME HANDLER to connect the update function to the Gtk loop for its computation
gboolean time_handler (gpointer data)
	{
	update_lattice(data);
	return TRUE;
	}

// Función genera los parámetros iniciales de la simulación
static void init_lattice(gpointer data)
	{
	int r=2;
	//Start with en empty lattice
	int x,y;
	for (x = 0; x < X_SIZE; x++)
    	{
        for (y = 0; y < Y_SIZE; y++)
            {
            s.lattice_configuration[x][y]=0;	//Crea un latice vacío (lleno de 0s)
            }
        }
	s.occupancy = 0;
	s.lattice_configuration[X_SIZE/2][Y_SIZE/2]=1;	//Crea una partícula inicial al medio
	s.occupancy ++;
	s.initialized = TRUE;
	s.generation_time = 0;
	//paint_lattice(data);
	g_print ("Lattice initialized\n");
	}

//Función que inicia la simulación
static void start_simulation (gpointer data)
	{
	s.run = g_idle_add((GSourceFunc) (GSourceFunc) time_handler,GTK_IMAGE(data));  
	s.running = TRUE;
	g_print ("Simulation started\n");
	}

static gboolean destroy (gpointer userData) 
    {
    gtk_widget_destroy (userData);
    return FALSE;
    }

// ACTIVATE function with all Widget Initialization and creation
static void activate (GtkApplication *app, gpointer user_data)
	{	
	GtkWidget *window,*grid, *button, *logo_kimero_lab, *image_lattice;	//declare a bunch of Gtk WIDGETS for the GUI
	GdkPixbuf *pixbuf;							//to draw into the window images
	unsigned long long seed = (unsigned int)time(NULL); 			//we initialize the mt algorithm for random number genration
	init_genrand64(seed);
	window = gtk_application_window_new (app); 				//Create a new WINDOW
	gtk_window_set_title (GTK_WINDOW (window), "Ising Model"); 		//Set its title
	gtk_window_set_resizable (GTK_WINDOW(window), FALSE);			//Resizablec= False
	grid = gtk_grid_new (); 						//Here we make a GRID that is going pack our Widgets
	gtk_container_add (GTK_CONTAINER (window), grid); 			//Pack the GRID  in the window
	pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8, X_SIZE, Y_SIZE);     	//Pixel Buffer @ start up and Lattice configuration display image
	image_lattice = gtk_image_new_from_pixbuf(pixbuf);
	paint_a_background(image_lattice);
	gtk_grid_attach (GTK_GRID (grid), image_lattice, 0, 1, 5, 1); 		//position (0,1) spanning 5 col and 1 raw) 
	init_lattice (image_lattice); 						//Llama a la función que genera los parámetros iniciales
	start_simulation (image_lattice);					//Llama a la función que inicia la simulación
	g_timeout_add_seconds (80, destroy, window);
	//gtk_widget_show_all (window);						//Show the window and all widgets
	}

// Main 
int main (int argc, char **argv)
	{
    GtkApplication *app;
    int status;
	for (T = 2.27; T < 2.27; T = T + 0.01)
    	{
    	//int K;
    	for	(s.K = 1; s.K < 11; s.K++)
        	{
        	app = gtk_application_new ("keymer.lab.contact_process", G_APPLICATION_FLAGS_NONE);
        	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
        	status = g_application_run (G_APPLICATION (app), argc, argv);
        	g_object_unref (app);
        	}
        if (s.K == 11)
            {
            s.K = 1;
            }
    	}
	return status;
	}

// Implementation of putpixel. Thanks to code from,
// https://developer.gnome.org/gdk-pixbuf/stable/gdk-pixbuf-The-GdkPixbuf-Structure.html
void put_pixel(GdkPixbuf *pixbuf, int x, int y, guchar red, guchar green, guchar blue, guchar alpha)
	{
	guchar *pixels, *p;
	int rowstride, numchannels;
	numchannels = gdk_pixbuf_get_n_channels(pixbuf);
	rowstride = gdk_pixbuf_get_rowstride(pixbuf);
	pixels = gdk_pixbuf_get_pixels(pixbuf);
	p = pixels + y * rowstride + x * numchannels;
	p[0] = red;	p[1] = green; p[2] = blue; p[3] = alpha;
	}

