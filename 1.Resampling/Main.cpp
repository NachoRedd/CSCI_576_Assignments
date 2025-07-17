#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

using namespace std;
namespace fs = std::filesystem;

/**
 * Display an image using WxWidgets.
 * https://www.wxwidgets.org/
 */

/** Declarations*/

/**
 * Class that implements wxApp
 */
class MyApp : public wxApp {
 public:
  bool OnInit() override;
};

/**
 * Class that implements wxFrame.
 * This frame serves as the top level window for the program
 */
class MyFrame : public wxFrame {
 public:
  MyFrame(const wxString &title, string imagePath, int inWidth, int inHeight, int outWidth, int outHeight);

 private:
  void OnPaint(wxPaintEvent &event);
  wxImage inImage;
  wxScrolledWindow *scrolledWindow;
  int width;
  int height;
};

/** Utility function to read image data */
unsigned char *readImageData(string imagePath, int width, int height, int outWidth, int outHeight);

/**inData function**/
unsigned char* transferInData(vector<unsigned char> red, vector<unsigned char> green, vector<unsigned char> blue, int outWidth, int outHeight);
/**Downsampling Functions**/
//Create 2D vector of R/G/B stream(Functional for downsizing)
vector<vector<unsigned char>> to2D(vector<char> buf, int height, int width);
//Create 1D kernel
vector<vector<double>> create2DKernel(int kernelSize);
//Apply Kernel
vector<vector<unsigned char>> applyKernel(vector<vector<unsigned char>> image2D, vector<vector<double>> kernel, int kernelSize, int height, int width);
//ScaleDownO12
vector<vector<unsigned char>> scaleDownO12(vector<vector<unsigned char>> input, int height, int width, int outHeight, int outWidth);
//ScaleDownO3
vector<vector<unsigned char>> scaleDownO3(vector<vector<unsigned char>> input, int height, int width, int outHeight, int outWidth);
/**Upsample Using Bilinear Resizing**/
vector<vector<unsigned char>> scaleUp(vector<vector<unsigned char>> input, int height, int width, int outHeight, int outWidth);
//2D output to 1D stream
vector<unsigned char> to1D(vector<vector<char>> output2D, int height, int width);

//Gaussian Kernel Function
//Function to downsize

//Function to upsize

/** Definitions */

/**
 * Init method for the app.
 * Here we process the command line arguments and
 * instantiate the frame.
 */
bool MyApp::OnInit() {
  wxInitAllImageHandlers();

  // deal with command line arguments here
  //Need 5 inputs total --> FIlename, Width, Height, Resampling Meothd(1or2), OutputFormat(O1,O2,O3)
  //O1 --> 1920x1080, O2 --> 1280x720, O3 --> 640x480

  //Make sure argc != 5
  cout << "Number of command line arguments: " << wxApp::argc << endl;
  if (wxApp::argc != 5) {
    cerr << "The executable should be invoked with exactly one filepath "
            "argument. Example ./MyImageApplication '../../Lena_512_512.rgb' 512 512 O1"
         << endl;
    exit(1);
  }
  cout << "First argument: " << wxApp::argv[0] << endl;
  cout << "Second argument: " << wxApp::argv[1] << endl;
  string imagePath = wxApp::argv[1].ToStdString();
  //New Arguments
  //Width
  cout << "Third argument: " << wxApp::argv[2] << endl;
  int inWidth = wxAtoi(argv[2]);
  if (!(inWidth == 4000 || inWidth == 400)){
    cerr << "Third argument not 4000 or 400 Exiting..." << endl;
    exit(1);
  }
  //Height
  cout << "Fourth argument: " << wxApp::argv[3] << endl;
  int inHeight = wxAtoi(argv[3]);
  if (!(inHeight == 3000 || inHeight == 300)){
    cerr << "Fourth argument not 3000 or 300 Exiting..." << endl;
    exit(1);
  }

  if(!((inWidth == 4000 && inHeight == 3000) || (inWidth == 400 && inHeight == 300))){
    cerr << "Resolution not valid Exiting..." << endl;
    exit(1);
  }

  //Output Format
  cout << "Fifth argument: " << wxApp::argv[4] << endl;
  int outWidth;
  int outHeight;
  if (wxApp::argv[4] == "O1"){
    outWidth = 1920;
    outHeight = 1080;
  } else if (wxApp::argv[4] == "O2"){
    outWidth = 1280;
    outHeight = 720;
  } else if (wxApp::argv[4] == "O3"){
    outWidth = 640;
    outHeight = 480;
  } else {
    cerr << "Fifth argument not O1, O2, or O3. Exiting..." << endl;
    exit(1);
  }

  MyFrame *frame = new MyFrame("Image Display", imagePath, inWidth, inHeight, outWidth, outHeight);
  frame->Show(true);

  // return true to continue, false to exit the application
  return true;
}

/**
 * Constructor for the MyFrame class.
 * Here we read the pixel data from the file and set up the scrollable window.
 */
MyFrame::MyFrame(const wxString &title, string imagePath, int inWidth, int inHeight, int outWidth, int outHeight)
    : wxFrame(NULL, wxID_ANY, title) {

  // Modify the height and width values here to read and display an image with
  // different dimensions.
  //Change based on cout/argv inputs and arguements
  //This width and height should be final window size
  width = outWidth;
  height = outHeight;

  //Switch this to outWidth/outHeight or not
  unsigned char *inData = readImageData(imagePath, inWidth, inHeight, outWidth, outHeight);

  // the last argument is static_data, if it is false, after this call the
  // pointer to the data is owned by the wxImage object, which will be
  // responsible for deleting it. So this means that you should not delete the
  // data yourself.
  inImage.SetData(inData, width, height, false);

  // Set up the scrolled window as a child of this frame
  scrolledWindow = new wxScrolledWindow(this, wxID_ANY);
  scrolledWindow->SetScrollbars(10, 10, width, height);
  scrolledWindow->SetVirtualSize(width, height);

  // Bind the paint event to the OnPaint function of the scrolled window
  scrolledWindow->Bind(wxEVT_PAINT, &MyFrame::OnPaint, this);

  // Set the frame size
  SetClientSize(width, height);

  // Set the frame background color
  SetBackgroundColour(*wxBLACK);
}

/**
 * The OnPaint handler that paints the UI.
 * Here we paint the image pixels into the scrollable window.
 */
void MyFrame::OnPaint(wxPaintEvent &event) {
  wxBufferedPaintDC dc(scrolledWindow);
  scrolledWindow->DoPrepareDC(dc);

  wxBitmap inImageBitmap = wxBitmap(inImage);
  dc.DrawBitmap(inImageBitmap, 0, 0, false);
}

/** Function to convert 1D stream of color to 2D vector**/
vector<vector<unsigned char>> to2D(vector<char> buf, int height, int width){
  vector<vector<unsigned char>> image2D(height, vector<unsigned char>(width));
  for (int i = 0; i < height; i++){
    for (int j = 0; j < width; j++){
      int pixelIndex = (i * width + j);
      image2D[i][j] = buf[pixelIndex];
    }
  }
  //cout << "first pixel to2D: " << image2D[0][0] << endl;
  //cout << "first pixel red: " << buf[0] << endl;
  return image2D;
}
/** Function to create a 2D kernel **/
double gaussian( double x, double mu, double sigma ) {
    const double a = ( x - mu ) / sigma;
    return exp( -0.5 * a * a );
}
vector<vector<double>> create2DKernel(int kernelSize){
typedef vector<double> kernel_row;
typedef vector<kernel_row> kernel_type;
  double kernelRadius = kernelSize/2;
  double sigma = kernelRadius/2.;
  kernel_type kernel2d(2*kernelRadius+1, kernel_row(2*kernelRadius+1));
  double sum = 0;
  // compute values
  for (int row = 0; row < kernel2d.size(); row++)
    for (int col = 0; col < kernel2d[row].size(); col++) {
      double x = gaussian(row, kernelRadius, sigma) * gaussian(col, kernelRadius, sigma);
      kernel2d[row][col] = x;
      sum += x;
    }
  // normalize
  for (int row = 0; row < kernel2d.size(); row++){
    for (int col = 0; col < kernel2d[row].size(); col++){
      kernel2d[row][col] /= sum;
    }
  }
  cout<<"Created 2D kernel of size: " << kernelSize <<endl;   
  return kernel2d;
}
/** Function to apply kernel**/
vector<vector<unsigned char>> applyKernel(vector<vector<unsigned char>> image2D,vector<vector<double>> kernel, int kernelSize, int height,int width) {
  vector<vector<unsigned char>> output(height, vector<unsigned char>(width));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double sum = 0.0;
            for (int i = -1 * (kernelSize / 2); i <= (kernelSize / 2); i++) {
                for (int j = -1 * (kernelSize / 2); j <= (kernelSize / 2); j++) {
                    int sourceY = y + i;
                    int sourceX = x + j;

                    // Mirror border pixels
                    if (sourceY < 0) {
                        sourceY = 0;
                    } else if (sourceY >= height) {
                        sourceY =  height - 1;
                        //sourceY = 2 * height - sourceY - 2; 
                    }
                    if (sourceX < 0) {
                        sourceX = 0;
                    } else if (sourceX >= width) {
                        sourceX =  width - 1;
                        //sourceX = 2 * width - sourceX - 2;
                    }
                    sum += static_cast<int>(image2D[sourceY][sourceX]) * kernel[i + (kernelSize / 2)][j + (kernelSize / 2)];
                }
                output[y][x] = static_cast<unsigned char>(sum);
            }
        }
    }
    cout<<"Applied Gaussian Kernel to Red/Green/Blue Channel"<<endl;
    return output;
}
/**Downsample O1&O2**/
vector<vector<unsigned char>> scaleDownO12(vector<vector<unsigned char>> input, int height, int width, int outHeight, int outWidth){
  vector<vector<unsigned char>> output(outHeight, vector<unsigned char>(outWidth));
  double power = 10;
  double xStep = static_cast<double> (width) / outWidth;
  double yStep =  static_cast<double> (height) / outHeight;

  double xCenter = outWidth/2;
  double yCenter = outHeight/2;

  double ogX;
  double ogY;
  double nonLinearStretchX;
  double nonLinearStretchY;

  for (double y = 0; y < outHeight; y++) {
    for (double x = 0; x < outWidth; x++) {
      double normalDistX = abs(x - xCenter) / xCenter;
      double normalDistY = abs(y - yCenter) / yCenter;
      
      nonLinearStretchX = tanh(normalDistX*0.5) * power;
      nonLinearStretchY = tanh(normalDistY*0.5) * power;
      //nonLinearStretchX = (x/(1+abs(x)));
      //nonLinearStretchY = (y/(1+abs(y)));

      //cout<<"x: "<< x << " --- y: " << y << endl;
      //cout<<"stetchX "<< nonLinearStretchY << " --- stetchY: " << nonLinearStretchY << endl;
      ogX = (nonLinearStretchX + x) * xStep;
      ogY = (nonLinearStretchY + y) * yStep;

      ogX = clamp(ogX, 0.0, width - 1.0);
      ogY = clamp(ogY, 0.0, height - 1.0);

      output[y][x] = input[ogY][ogX];  
    }
  }
  cout<<"Down Sampled Red/Green/Blue Channel"<<endl;
  return output;
}
/**Downsample O3**/
vector<vector<unsigned char>> scaleDownO3(vector<vector<unsigned char>> input, int height, int width, int outHeight, int outWidth){
  vector<vector<unsigned char>> output(outHeight, vector<unsigned char>(outWidth));
    int stepX = width / outWidth;
    int stepY = height / outHeight;

    for (int y = 0; y < outHeight; y++) {
        for (int x = 0; x < outWidth; x++) {
            int ogX = x * stepX;
            int ogY = y * stepY;
            output[y][x] = input[ogY][ogX];
        }
    }
    cout<<"Down Sampled Red/Green/Blue Channel"<<endl;
    return output;
}
/**Upsample Using Bilinear Resizing**/
vector<vector<unsigned char>> scaleUp(vector<vector<unsigned char>> input, int height, int width, int outHeight, int outWidth){
  vector<vector<unsigned char>> output(outHeight, vector<unsigned char>(outWidth));
  float xRatio = static_cast<float>(width-1) / (outWidth - 1);
  float yRatio = static_cast<float>(height-1) / (outHeight - 1);

    for (int y = 0; y < outHeight; y++) {
        for (int x = 0; x < outWidth; x++) {
          float x1 = floor(xRatio * x);
          float y1 = floor(yRatio * y);
          float xh = ceil(xRatio * x);
          float yh = ceil(yRatio * y);

          float xWeight = (xRatio * x) - x1;
          float yWeight = (yRatio * y) - y1;

          unsigned char a = (input[y1][x1]);
          unsigned char b = (input[y1][xh]);
          unsigned char c = (input[yh][x1]);
          unsigned char d = (input[yh][xh]);

          float pixel = a * (1 - xWeight) * (1 - yWeight) + b * (xWeight) * (1 - yWeight) + c * (yWeight) * (1 - xWeight) + d * (xWeight) * (yWeight);

          output[y][x] = static_cast<unsigned char>((pixel));
        }
    }
    cout<<"Up Sampled Red/Green/Blue Channel"<<endl;
    return output;
}
/** Function to turn 2D back into readable 1D stream**/
vector<unsigned char> to1D(vector<vector<unsigned char>> output2D, int height, int width){
  vector<unsigned char> buf(width * height);
  int index = 0;
  for (int i = 0; i < height; i++){
    for (int j = 0; j < width; j++){
      buf[index] = static_cast<unsigned char>(output2D[i][j]);
      index++;
      //cout << "Current buf: " << static_cast<unsigned char>(output2D[i][j]) << "--------- j: " << j << " i: " << i << endl;
    }
  }
  return buf;
}
/**Function to transfer to inData**/
unsigned char *transferInData(vector<unsigned char> red, vector<unsigned char> green, vector<unsigned char> blue, int outWidth, int outHeight){
  /**
   * Allocate a buffer to store the pixel values
   * The data must be allocated with malloc(), NOT with operator new. wxWidgets
   * library requires this.
   */
  unsigned char *inData = (unsigned char *)malloc(outWidth * outHeight * 3 * sizeof(unsigned char));
      
  for (int i = 0; i < outHeight * outWidth; i++) {
    // We populate RGB values of each pixel in that order
    // RGB.RGB.RGB and so on for all pixels
    inData[3 * i] = red[i];
    inData[3 * i + 1] = green[i];
    inData[3 * i + 2] = blue[i];
    //cout << "Red: " << redBlur[i] << "---Green: " << greenBlur[i] << "---Blue: " << blueBlur[i] <<endl;
  }
  return inData;
}

/** Utility function to read image data */
unsigned char *readImageData(string imagePath, int width, int height, int outWidth, int outHeight) {

  // Open the file in binary mode
  ifstream inputFile(imagePath, ios::binary);

  if (!inputFile.is_open()) {
    cerr << "Error Opening File for Reading" << endl;
    exit(1);
  }

  // Create and populate RGB buffers
  vector<char> Rbuf(width * height);
  vector <char> Gbuf(width * height);
  vector<char> Bbuf(width * height);

  /**
   * The input RGB file is formatted as RRRR.....GGGG....BBBB.
   * i.e the R values of all the pixels followed by the G values
   * of all the pixels followed by the B values of all pixels.
   * Hence we read the data in that order.
   */

  inputFile.read(Rbuf.data(), width * height);
  inputFile.read(Gbuf.data(), width * height);
  inputFile.read(Bbuf.data(), width * height);
  inputFile.close();

  int kernelSize = 5;
  if (outWidth < width){
    cout << "Time to DownSample"<<endl;
    if (outWidth == 1920 || outWidth == 1280){
    cout<<"O1 or O2 Selected"<<endl;
    //Turn to 2D array
    vector<vector<unsigned char>> red2D = to2D(Rbuf, height, width);
    vector<vector<unsigned char>> green2D = to2D(Gbuf, height, width);
    vector<vector<unsigned char>> blue2D = to2D(Bbuf, height, width);

    //Now I can use a 2D Gaussian Kernel
    vector<vector<double>> kernel2D = create2DKernel(kernelSize);
 
    //Apply Kernel Horizontal & Vertical for Red
    vector<vector<unsigned char>> outputRed = applyKernel(red2D, kernel2D, kernelSize, height, width);
    //Apply Kernel Horizontal & Vertical for Green
    vector<vector<unsigned char>> outputGreen = applyKernel(green2D, kernel2D, kernelSize, height, width);
    //Apply Kernel Horizontal & Vertical for Blue
    vector<vector<unsigned char>> outputBlue = applyKernel(blue2D, kernel2D, kernelSize, height, width);
    //Scale Down
    vector<vector<unsigned char>> smallRed = scaleDownO12(outputRed, height, width, outHeight, outWidth);
    vector<vector<unsigned char>> smallGreen = scaleDownO12(outputGreen, height, width, outHeight, outWidth);
    vector<vector<unsigned char>> smallBlue = scaleDownO12(outputBlue, height, width, outHeight, outWidth);

    //Turn back into stream
    vector<unsigned char> redBlur = to1D(smallRed, outHeight, outWidth);
    vector<unsigned char> greenBlur = to1D(smallGreen, outHeight, outWidth);
    vector<unsigned char> blueBlur = to1D(smallBlue, outHeight, outWidth);

    //Finish
    return transferInData(redBlur, greenBlur, blueBlur, outWidth, outHeight);

  } else if (outWidth == 640){
    cout<<"O3 Selected"<<endl;
    //Turn to 2D array
    vector<vector<unsigned char>> red2D = to2D(Rbuf, height, width);
    vector<vector<unsigned char>> green2D = to2D(Gbuf, height, width);
    vector<vector<unsigned char>> blue2D = to2D(Bbuf, height, width);
  
    //Now I can use a 2D Gaussian Kernel
    vector<vector<double>> kernel2D = create2DKernel(kernelSize);
 
    //Apply Kernel Horizontal & Vertical for Red
    vector<vector<unsigned char>> outputRed = applyKernel(red2D, kernel2D, kernelSize, height, width);
    //Apply Kernel Horizontal & Vertical for Green
    vector<vector<unsigned char>> outputGreen = applyKernel(green2D, kernel2D, kernelSize, height, width);
    //Apply Kernel Horizontal & Vertical for Blue
    vector<vector<unsigned char>> outputBlue = applyKernel(blue2D, kernel2D, kernelSize, height, width);

  //Scale Down
    vector<vector<unsigned char>> smallRed = scaleDownO3(outputRed, height, width, outHeight, outWidth);
    vector<vector<unsigned char>> smallGreen = scaleDownO3(outputGreen, height, width, outHeight, outWidth);
    vector<vector<unsigned char>> smallBlue = scaleDownO3(outputBlue, height, width, outHeight, outWidth);

  //Turn back into stream
    vector<unsigned char> redBlur = to1D(smallRed, outHeight, outWidth);
    vector<unsigned char> greenBlur = to1D(smallGreen, outHeight, outWidth);
    vector<unsigned char> blueBlur = to1D(smallBlue, outHeight, outWidth);

    //Finish
    return transferInData(redBlur, greenBlur, blueBlur, outWidth, outHeight);
    } else {
      cout << "How did you get here"<<endl;
      return NULL;
    }
    } else if (outWidth > width){
    //upSample
    //Turn to 2D array
    vector<vector<unsigned char>> red2D = to2D(Rbuf, height, width);
    vector<vector<unsigned char>> green2D = to2D(Gbuf, height, width);
    vector<vector<unsigned char>> blue2D = to2D(Bbuf, height, width);

    //Bilinear Resize
    vector<vector<unsigned char>> bigRed = scaleUp(red2D, height, width, outHeight, outWidth);
    vector<vector<unsigned char>> bigGreen = scaleUp(green2D, height, width, outHeight, outWidth);
    vector<vector<unsigned char>> bigBlue = scaleUp(blue2D, height, width, outHeight, outWidth);

    //Turn back into stream
    vector<unsigned char> redBlur = to1D(bigRed, outHeight, outWidth);
    vector<unsigned char> greenBlur = to1D(bigGreen, outHeight, outWidth);
    vector<unsigned char> blueBlur = to1D(bigBlue, outHeight, outWidth);

    //Finish
    return transferInData(redBlur, greenBlur, blueBlur, outWidth, outHeight);

  } else {
    cout<<"Something went wrong, you shouldn't be here"<<endl;
    return NULL;
  }
  return NULL;
}

wxIMPLEMENT_APP(MyApp);