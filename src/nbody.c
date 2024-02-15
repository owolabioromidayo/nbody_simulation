#include "nbody.h"


bool USE_OPENMPI = 0;
int NUM_BODIES = 0;
double DT = 0.0;
int USE_ORBITAL =0;
DisplayData dd;

bool simulationRunning = true; 

////////////////// FPS ////////////// 
struct timeval lastTime;
int frameCount = 0;
float fps = 0.0;

void updateFPS() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    double elapsed = (currentTime.tv_sec - lastTime.tv_sec) +
                     (currentTime.tv_usec - lastTime.tv_usec) / 1.0e6;

    frameCount++;

    if (elapsed > 1.0) {
        fps = frameCount / elapsed;
        // printf("FPS: %.2f\n", fps);

        // Reset frame count and last update time
        frameCount = 0;
        lastTime = currentTime;
    }
}

//////////////////////////////// MOTION /////////////////////////


///// INIT CAMERA VARS
float cameraX = 0.0;
float cameraY = 0.0;
float cameraZ = 0.0;

float angleX = 0.0;
float angleY = 0.0;

int startX = 0;
int startY = 0;



void motion(int x, int y) {
    // Calculate mouse movement
    int deltaX = x - startX;
    int deltaY = y - startY;

    int MAX = 1000;

    if (deltaX > MAX)
        deltaX = MAX;
    
    else if (deltaX < -MAX)
        deltaX = -MAX;
    
    if (deltaY > MAX)
        deltaY = MAX;
    
    else if (deltaY < -MAX)
        deltaY = -MAX;

    // printf("Changes : %d %d \n", deltaX, deltaY); 
    // Update rotation angles based on mouse movement
    // inversion actually works here
    angleX += deltaY * 0.1;
    angleY += deltaX * 0.1;

    // Update start position for next motion event
    startX = x;
    startY = y;

   
    glutPostRedisplay();
}

// Function to handle keyboard input for adjusting camera position
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 's':
            cameraY -= 0.1;
            break;
        case 'w':
            cameraY += 0.1;
            break;
        case 'a':
            cameraX -= 0.1;
            break;
        case 'd':
            cameraX += 0.1;
            break;
        case 'q':
            cameraZ -= 0.1;
            break;
        case 'e':
            cameraZ += 0.1;
            break;
    }
    glutPostRedisplay();
}

/////////////////////////////////////////////////////


void initColors(){
    for( int i =0; i < NUM_BODIES; i++){
        dd.body_colors[i*3 + 0]= (rand() % 1000) / 1000.0;
        dd.body_colors[i*3 + 1] = (rand() % 1000) / 1000.0;
        dd.body_colors[i*3 + 2]= (rand() % 1000) / 1000.0;


        // for cyan
        // body_colors[i][0]= 0;
        // body_colors[i][1] = 1;
        // body_colors[i][2]= 1;
    }
}

void initBodies() {
    //randomize
    for (int i = 0; i < NUM_BODIES; i++) {
        dd.bodies[i].x = (rand() % 2000 -1000) / 1000.0; //includes negative range
        dd.bodies[i].y = (rand() % 2000 -1000) / 1000.0;
        dd.bodies[i].z = (rand() % 2000 -1000) / 1000.0;
        dd.bodies[i].vx = (rand() % 2000 - 1000) / 1000.0;
        dd.bodies[i].vy = (rand() % 2000 - 1000) / 1000.0;
        dd.bodies[i].vz = (rand() % 2000 - 1000) / 1000.0;
        dd.bodies[i].mass = (rand() % 100000 + 1000) / 1000.0;
    }
}


void initOrbitalBodies() {
    // Set the mass and position of the central body 
    dd.bodies[0].mass = STAR_MASS ;
    dd.bodies[0].x = 0.0;
    dd.bodies[0].y = 0.0;
    dd.bodies[0].z = 0.0;

    // Set the velocities of the other bodies in circular or elliptical orbits
    double orbitRadius = 0.5; // Radius of the orbit
    double orbitalSpeed = ORBITAL_SPEED_CONSTANT * sqrt(G * dd.bodies[0].mass / orbitRadius); // Calculate orbital speed
    double angleIncrement = 2 * M_PI / (NUM_BODIES - 1); // Angle increment for each body

    #if USE_OPENMP
    #pragma omp parallel for
    #endif
    for (int i = 1; i < NUM_BODIES; i++) {
        dd.bodies[i].mass = rand() % 400 + 100;
        // Set the positions in a circular or elliptical orbit
        double angle = angleIncrement * (i - 1);
        // dd.bodies[i].x = 0.5 * sqrt(i) * orbitRadius * cos(angle);
        // dd.bodies[i].y = 0.5* sqrt(i) *orbitRadius * sin(angle);
        // dd.bodies[i].z = 0.0; // For simplicity, assume all orbits are in the xy-plane
        dd.bodies[i].x = (rand() % 2000 -1000) / 1000.0; //includes negative range
        dd.bodies[i].y = (rand() % 2000 -1000) / 1000.0;
        dd.bodies[i].z = (rand() % 2000 -1000) / 1000.0;

        // Set the velocities perpendicular to the position vector to ensure circular motion
        dd.bodies[i].vx = -orbitalSpeed * sin(angle);
        dd.bodies[i].vy = orbitalSpeed * cos(angle);
        dd.bodies[i].vz = 0.0;
    }
}

// Function to update positions and velocities of bodies
void updateBodies() {
    #if USE_OPENMP
    #pragma omp parallel for
    #endif
    for (int i = 1; i < NUM_BODIES; i++) { // skip the main
        // Update position using velocity
        dd.bodies[i].x += dd.bodies[i].vx * DT;
        dd.bodies[i].y += dd.bodies[i].vy * DT;
        dd.bodies[i].z += dd.bodies[i].vz * DT;

        #if SHOW_PATHS
            if (dd.numPoints[i] < MAX_POINTS) {
                dd.paths[i*MAX_POINTS + dd.numPoints[i]].x = dd.bodies[i].x;
                dd.paths[i*MAX_POINTS + dd.numPoints[i]].y = dd.bodies[i].y;
                dd.paths[i*MAX_POINTS + dd.numPoints[i]].z = dd.bodies[i].z;
                dd.numPoints[i]++;
            }
            // when the buffer is full, reset
            else {
                // a shiftiing operation is O(n) unfortunately. maybe just reset
                dd.numPoints[i] = 0;
            }

        #endif     

        // calc accel
        double ax = 0.0, ay = 0.0, az = 0.0;
        #if USE_OPENMP
        #pragma omp parallel for
        #endif
        for (int j = 0; j < NUM_BODIES; j++) {
            if (i != j) {
                double dx = dd.bodies[j].x - dd.bodies[i].x;
                double dy = dd.bodies[j].y - dd.bodies[i].y;
                double dz = dd.bodies[j].z - dd.bodies[i].z;
                double r = sqrt(dx * dx + dy * dy + dz * dz); 
                double f = G * dd.bodies[i].mass * dd.bodies[j].mass / (r * r); 
                ax += f * dx / r; 
                ay += f * dy / r;
                az += f * dz / r;

                // printf("r : %f, dx: %f, dy: %f dz: %f, f %f \n", r, dx, dy,dz,f);
                // printf("%d | Change in ax %f %f %f \n", i, ax, ay, az );
            }
        }

        // printf("%d | V %f %f %f | A %f %f %f \n", i, bodies[i].vx, bodies[i].vy, bodies[i].vz, ax, ay, az);

        
        dd.bodies[i].vx += ax * DT;
        dd.bodies[i].vy += ay * DT;
        dd.bodies[i].vz += az * DT;
    }
}


#if SHOW_PATHS
void drawPaths() {
    glLineWidth(1.0); 
    
    for (int i = 0; i < NUM_BODIES; i++) {
        glColor3f(dd.body_colors[i *3 + 0], dd.body_colors[i*3 + 1], dd.body_colors[i*3 + 2]); // light gray
        glBegin(GL_LINE_STRIP); 
        for (int j = 0; j < dd.numPoints[i]; j++) 
            glVertex3f(dd.paths[i * MAX_POINTS +j].x, dd.paths[i * MAX_POINTS+ j].y, dd.paths[i * MAX_POINTS +j].z);
        glEnd(); 
    }
}
#endif

// Function to draw bodies
void drawBodies() {
    // glColor3f(1.0, 1.0, 1.0); // white
    for (int i = 0; i < NUM_BODIES; i++) {
        glColor3f(dd.body_colors[i*3 + 0], dd.body_colors[i*3 + 1], dd.body_colors[i*3 + 2]); 
        glPushMatrix(); // Save current transformation matrix
        glTranslatef(dd.bodies[i].x, dd.bodies[i].y, dd.bodies[i].z); // move to pos
        glutSolidSphere(0.0005 * 2  * pow(log(dd.bodies[i].mass), 2 ) , 10, 10); 
        glPopMatrix(); // Restore previous transformation matrix
    }
}

void drawText(float x, float y, void* font, const char* format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[256];
    vsprintf(buffer, format, args);

    va_end(args);

    glPushMatrix();
    glLoadIdentity();
    glRasterPos2f(x, y);

    for (const char* c = buffer; *c != '\0'; ++c) {
        glutBitmapCharacter(font, *c);
    }

    glPopMatrix();
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
    glMatrixMode(GL_MODELVIEW); 
    glLoadIdentity(); 
    // gluLookAt(0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0); // Set up the camera position
    gluLookAt(0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // Set up the camera position
    
    // CAMERA UPDATE
    glRotatef(angleX, 1.0, 0.0, 0.0);
    glRotatef(angleY, 0.0, 1.0, 0.0);

    // Set camera position
    glTranslatef(-cameraX, -cameraY, -cameraZ);


    updateBodies();
    if (SHOW_PATHS)
        drawPaths(); 
    drawBodies(); 

    updateFPS();

    // Draw FPS on the screen
    glColor3f(1.0, 1.0, 1.0); // Set text color to white
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    drawText(10, 580, GLUT_BITMAP_HELVETICA_12, "FPS: %.2f", fps);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers(); // Swap front and back buffers to display rendered image
}

//window resizing
void reshape(int width, int height) {
    glViewport(0, 0, width, height); // Set viewport to cover entire window
    glMatrixMode(GL_PROJECTION); // Select projection matrix
    glLoadIdentity(); // Load identity matrix
    gluPerspective(45.0, (double)width / (double)height, 0.1, 100.0); // Set perspective projection
    glMatrixMode(GL_MODELVIEW); // Select modelview matrix
}

void update(int value) {
    if (simulationRunning) {
        glutPostRedisplay(); 
    }
    glutTimerFunc(16, update, 0); 
}

int main(int argc, char** argv) {

    // lets set flags from clargs

    // lets use 1 and 2 as DT and NUM_BODIES
    if (argc >= 4){
        DT = strtod(argv[1], NULL);
        NUM_BODIES = atoi(argv[2]);
        USE_ORBITAL = atoi(argv[3]);
    } else{
        printf("You did not provide values for DT, NUM_BODIES and USE_ORBITAL. \n");
        return -1;
    }

    printf("Running simulation with DT:%f and NUM_BODIES:%d. USE_ORBITAL:%d \n", DT, NUM_BODIES, USE_ORBITAL);

    Body bodies[NUM_BODIES]; // Array to store bodies
    double body_colors[NUM_BODIES][3]; 

    #if SHOW_PATHS
    Point paths[NUM_BODIES][MAX_POINTS]; // Array to store paths of each body
    int numPoints[NUM_BODIES]; // Number of points stored for each body
    for (int i =0; i < NUM_BODIES; i++)
        numPoints[i] = 0;
    dd.paths = &paths;
    dd.numPoints = &numPoints;

    #endif

    //init all the display data here
    dd.bodies = &bodies;
    dd.body_colors = &body_colors;

    srand(time(NULL)); // seed RNG

    if (USE_ORBITAL)
        initOrbitalBodies();
    else
        initBodies(); 

    initColors();

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // Enable double buffering, RGB color, and depth buffer
    glutInitWindowSize(800, 600); // Set initial window size
    glutCreateWindow("N-Body Simulation"); // Create window with specified title
    glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D rendering
    glClearColor(0.0, 0.0, 0.0, 1.0); // Set clear color to black


     glutSetCursor(GLUT_CURSOR_INFO); // glove

    // callback register
    glutDisplayFunc(display); 
    glutReshapeFunc(reshape);
    glutMotionFunc(motion); 
    glutKeyboardFunc(keyboard);

    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}
