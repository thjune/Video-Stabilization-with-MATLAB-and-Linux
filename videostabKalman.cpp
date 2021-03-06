#include <opencv2/opencv.hpp>     //Paquete de OpenCV en ROS para C++.
#include <opencv2/features2d.hpp> //Paquete de OpenCV para deteccion y descripcion de
				  //puntos de interes de imagenes en ROS para C++
#include <iostream>               //Define entrada/salida de
				  //transmision en tiempo real.
#include <cassert>		  //Define una macro funcion
				  //que puede ser usado como una
				  //herramienta standard para eliminar fallas.
#include <cmath>		  //Declara  funciones establecidas para
				  //calculo de operaciones y transformaciones
				  //matematicas.
#include <fstream>		  //Clase que opera archivos de entrada/salida
				  //de transmision en tiempo real
using namespace std;		  //Prefijo standard establecido para
				  //optimizar el codigo de programacion
using namespace cv;		  //Prefijo de vision por computadora
				  //establecido para optimizar el codigo de
			          //programacion

//Esta estabilizacion de video, suaviza la trayectoria global usando una
//ventana que se desliza regularmente

//const int SMOOTHING_RADIUS = 15; // En fotogramas. Mientras mas grande el
//valor, mas estable el video, pero menos reactivo para inclinaciones
//repentinas
const int HORIZONTAL_BORDER_CROP = 60; // En pixeles. Corta el borde para
//reducir los bordes negros, previniendo que la estabilizacion sea muy
//notable.

// 1. Obtener la transformacion previa y actual de los fotogramas (dx(coordenada), dy(coordenada),
//da(angulo)) para todos los fotogramas
// 2. Acumular las transformaciones para obtener la trayectoria de la imagen
// 3. Suavizar la trayectoria usando una ventana que se deslice regularmente
// 4. Generar una nueva asignacion de transformaciones previas a actuales, tales que la trayectoria
//termine siendo la mismo que la trayectoria suavizada
// 5. Aplicar la nueva transformacion al video
//void run();

struct TransformParam					//
{
    TransformParam() {}
    TransformParam(double _dx, double _dy, double _da) {
        dx = _dx;
        dy = _dy;
        da = _da;
    }

    double dx; //traslacion x
    double dy; //traslacion y
    double da; // angle
};

struct Trajectory
{
    Trajectory() {}
    Trajectory(double _x, double _y, double _a) {
        x = _x;
        y = _y;
        a = _a;
    }
	// "+"
	friend Trajectory operator+(const Trajectory &c1,const Trajectory  &c2){
		return Trajectory(c1.x+c2.x,c1.y+c2.y,c1.a+c2.a);
	}
	//"-"
	friend Trajectory operator-(const Trajectory &c1,const Trajectory  &c2){
		return Trajectory(c1.x-c2.x,c1.y-c2.y,c1.a-c2.a);
	}
	//"*"
	friend Trajectory operator*(const Trajectory &c1,const Trajectory  &c2){
		return Trajectory(c1.x*c2.x,c1.y*c2.y,c1.a*c2.a);
	}
	//"/"
	friend Trajectory operator/(const Trajectory &c1,const Trajectory  &c2){
		return Trajectory(c1.x/c2.x,c1.y/c2.y,c1.a/c2.a);
	}
	//"="
	Trajectory operator =(const Trajectory &rx){
		x = rx.x;
		y = rx.y;
		a = rx.a;
		return Trajectory(x,y,a);
	}

    double x;
    double y;
    double a; // angle
};
//

/*void run()
{
        VideoWriter outputVideo;
        Mat stabilizedFrame;
        int nframes=0;
        while (!(stabilizedFrame=canvas->nextFrame()).empty())
        {
                nframes++;
                if(!outputPath.empty())
                {
                        if(!outputVideo.isOpened())
                        outputVideo.open(outputPath, CV_FOURCC('X','V','I','D')$
                outputVideo << canvas;
                }
        }
}*/

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "./VideoStab [video.avi]" << endl;
		return 0;
	}
	// For further analysis
	ofstream out_transform("prev_to_cur_transformation.txt");
	ofstream out_trajectory("trajectory.txt");
	ofstream out_smoothed_trajectory("smoothed_trajectory.txt");
	ofstream out_new_transform("new_prev_to_cur_transformation.txt");

	VideoCapture cap(argv[1]);
	assert(cap.isOpened());

	//Filtro de Kalman (imagen anterior(previous), imagen actual(current))
	Mat cur, cur_grey;
	Mat prev, prev_grey;
	//Captura de Fotograma
	cap >> prev;//get the first frame.ch
	cvtColor(prev, prev_grey, COLOR_BGR2GRAY);

	//Orb
	vector<KeyPoint> kp;
	vector<Point2f> kp_prev, kp_cur;

    // Default parameters of ORB
    int nfeatures=500;
    float scaleFactor=1.3f;
    int nlevels=8;
    int edgeThreshold=15; // Changed default (31);
    int firstLevel=0;
    int WTA_K=2;
    int scoreType=ORB::HARRIS_SCORE;
    int patchSize=31;
    int fastThreshold=20;

    Ptr<ORB> detector = ORB::create(
    nfeatures,
    scaleFactor,
    nlevels,
    edgeThreshold,
    firstLevel,
    WTA_K,
    scoreType,
    patchSize,
    fastThreshold );

    detector->detect(prev, kp);
    KeyPoint::convert(kp,kp_prev);
//    cout << kp_prev.size()<<endl;

/*    fstream orb_keypoints;
    orb_keypoints.open( "orb_keypoints.txt", ios::out);
    for( size_t ii = 0; ii < kp.size(); ++ii )
	{
        orb_keypoints << kp[ii].pt.x << " " << kp[ii].pt.y << " "<<kp[ii].angle<<endl;
	}
    orb_keypoints.close( );
*/
	// Paso 1 - Tomar las transformaciones de fotogramas previos a actuales
	// (dx, dy, da) para todos los fotogramas
	vector <TransformParam> prev_to_cur_transform; // previous to current
	// Accumulated frame to frame transform
	double a = 0;
	double x = 0;
	double y = 0;
	// Step 2 - Accumulate the transformations to get the image trajectory
	vector <Trajectory> trajectory; // trajectory at all frames
	//
	// Step 3 - Smooth out the trajectory using an averaging window
	vector <Trajectory> smoothed_trajectory; // trajectory at all frames
	Trajectory X;//posteriori state estimate
	Trajectory	X_;//priori estimate
	Trajectory P;// posteriori estimate error covariance
	Trajectory P_;// priori estimate error covariance
	Trajectory K;//gain
	Trajectory	z;//actual measurement
	double pstd = 0.5;//can be changed//4e-3
	double cstd = 0.75;//can be changed//0.25
	Trajectory Q(pstd,pstd,pstd);// process noise covariance
	Trajectory R(cstd,cstd,cstd);// measurement noise covariance
	// Step 4 - Generate new set of previous to current transform, such that the trajectory ends up being the same as the smoothed trajectory
	vector <TransformParam> new_prev_to_cur_transform;
	//
	// Step 5 - Apply the new transformation to the video
	//cap.set(CV_CAP_PROP_POS_FRAMES, 0);
	Mat T(2,3,CV_64F); //Matriz de valores de pixeles de tipo double.

	int vert_border = HORIZONTAL_BORDER_CROP * prev.rows / prev.cols; // get the aspect ratio correct

	int k=1;
	int max_frames = cap.get(CV_CAP_PROP_FRAME_COUNT);
	Mat last_T;
	Mat prev_grey_,cur_grey_;

	while(true) {

		cap >> cur;
		if(cur.data == NULL) {
			break;
		}

		cvtColor(cur, cur_grey, COLOR_BGR2GRAY);

		// vector from prev to cur
		//vector <Point2f> prev_corner, cur_corner;
		vector <Point2f> prev_corner2, cur_corner2, prev_zero, cur_zero;
		vector <uchar> status;
		vector <float> err;

		//goodFeaturesToTrack(prev_grey, prev_corner, 200, 0.01, 30);
		//cout << "Found" << prev_corner.size() << "keypoints" << endl;
		detector->detect(prev_grey, kp);
    		KeyPoint::convert(kp,kp_prev);
		calcOpticalFlowPyrLK(prev_grey, cur_grey, kp_prev, kp_cur, status, err);

		// weed out bad matches
		for(size_t i=0; i < status.size(); i++) {
			if(status[i]) {
				prev_corner2.push_back(kp_prev[i]);
				cur_corner2.push_back(kp_cur[i]);
			}
		}

		// translation + rotation only
		Mat T = estimateRigidTransform(prev_corner2, cur_corner2, false); // false = rigid transform, no scaling/shearing

		// in rare cases no transform is found. We'll just use the last known good transform.
		if(T.data == NULL) {
			last_T.copyTo(T);
		}

		T.copyTo(last_T);

		// decompose T
		double dx = T.at<double>(0,2);
		double dy = T.at<double>(1,2);
		double da = atan2(T.at<double>(1,0), T.at<double>(0,0));
		//
		//prev_to_cur_transform.push_back(TransformParam(dx, dy, da));

		out_transform << k << " " << dx << " " << dy << " " << da << endl;
		//
		// Accumulated frame to frame transform
		x += dx;
		y += dy;
		a += da;
		//trajectory.push_back(Trajectory(x,y,a));
		//
		out_trajectory << k << " " << x << " " << y << " " << a << endl;
		//
		z = Trajectory(x,y,a);
		//
		if(k==1){
			// intial guesses
			X = Trajectory(0,0,0); //Initial estimate,  set 0
			P =Trajectory(1,1,1); //set error variance,set 1
		}
		else
		{
			//time update£¨prediction£©
			X_ = X; //X_(k) = X(k-1);
			P_ = P+Q; //P_(k) = P(k-1)+Q;
			// measurement update£¨correction£©
			K = P_/( P_+R ); //gain;K(k) = P_(k)/( P_(k)+R );
			X = X_+K*(z-X_); //z-X_ is residual,X(k) = X_(k)+K(k)*(z(k)-X_(k)); 
			P = (Trajectory(1,1,1)-K)*P_; //P(k) = (1-K(k))*P_(k);
		}
		//smoothed_trajectory.push_back(X);
		out_smoothed_trajectory << k << " " << X.x << " " << X.y << " " << X.a << endl;
		//-
		// target - current
		double diff_x = X.x - x;//
		double diff_y = X.y - y;
		double diff_a = X.a - a;

		dx = dx + diff_x;
		dy = dy + diff_y;
		da = da + diff_a;

		//new_prev_to_cur_transform.push_back(TransformParam(dx, dy, da));
		//
		out_new_transform << k << " " << dx << " " << dy << " " << da << endl;
		//
		T.at<double>(0,0) = cos(da);
		T.at<double>(0,1) = -sin(da);
		T.at<double>(1,0) = sin(da);
		T.at<double>(1,1) = cos(da);

		T.at<double>(0,2) = dx;
		T.at<double>(1,2) = dy;

		Mat cur2;

		warpAffine(prev, cur2, T, cur.size());

		cur2 = cur2(Range(vert_border, cur2.rows-vert_border), Range(HORIZONTAL_BORDER_CROP+10, cur2.cols-HORIZONTAL_BORDER_CROP-10));

		// Resize cur2 back to cur size, for better side by side comparison
		resize(cur2, cur2, cur.size());

		// Now draw the original and stablised side by side for coolness
		Mat canvas = Mat::zeros(cur.rows, cur.cols*2+10, cur.type());//cur.cols*2+10

		prev.copyTo(canvas(Range::all(), Range(0, cur2.cols)));
		cur2.copyTo(canvas(Range::all(), Range(cur2.cols+10, cur2.cols*2+10)));//cur2.copyTo(canvas(Range::all(), Range(cur2.cols+10, cur2.cols*2+10)));

		// If too big to fit on the screen, then scale it down by 2, hopefully it'll fit :)
		if(canvas.cols > 1920) {
			resize(canvas, canvas, Size(canvas.cols/2, canvas.rows/2));
		}
		//outputVideo<<canvas;
		imshow("before and after", canvas);

		waitKey(10);
		//
		prev = cur.clone();//cur.copyTo(prev);
		cur_grey.copyTo(prev_grey);

		cout << "Frame: " << k << "/" << max_frames << " - good optical flow: " << prev_corner2.size() << endl;
		k++;

	}
	return 0;
}

