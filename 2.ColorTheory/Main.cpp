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
  MyFrame(const wxString &title, string imagePath, int hue1, int hue2);

 private:
  void OnPaint(wxPaintEvent &event);
  wxImage inImage;
  wxScrolledWindow *scrolledWindow;
  int width;
  int height;
};

/** Utility function to read image data */
unsigned char *readImageData(string imagePath, int width, int height, int hue1, int hue2);
/**inData function**/
unsigned char* transferInData(vector<unsigned char> red, vector<unsigned char> green, vector<unsigned char> blue, int outWidth, int outHeight);

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

  cout << "Number of command line arguments: " << wxApp::argc << endl;
  if (wxApp::argc != 4) {
    cerr << "The executable should be invoked with exactly one filepath "
            "argument. Example ./MyImageApplication '../../Lena_512_512.rgb' 512 512 O1"
         << endl;
    exit(1);
  }
  cout << "First argument: " << wxApp::argv[0] << endl;
  cout << "Second argument: " << wxApp::argv[1] << endl;
  string imagePath = wxApp::argv[1].ToStdString();
  cout << "Third argument: " << wxApp::argv[2] << endl;
  int hue1 = wxAtoi(argv[2]);
  if ((hue1 < 0 || hue1 > 360)){
    cerr << "Third argument (hue1) not between 0-360 Exiting..." << endl;
    exit(1);
  }
  //Height
  cout << "Fourth argument: " << wxApp::argv[3] << endl;
  int hue2 = wxAtoi(argv[3]);
  if ((hue2 < 0 || hue2 > 360)){
    cerr << "Third argument (hue2) not between 0-360 Exiting..." << endl;
    exit(1);
  }
  if (hue1 > hue2){
    cerr << "hue1 is greater than hue 2 Exiting..." << endl;
    exit(1);
  }

  MyFrame *frame = new MyFrame("Image Display", imagePath, hue1, hue2);
  frame->Show(true);

  // return true to continue, false to exit the application
  return true;
}

/**
 * Constructor for the MyFrame class.
 * Here we read the pixel data from the file and set up the scrollable window.
 */
MyFrame::MyFrame(const wxString &title, string imagePath, int hue1, int hue2)
    : wxFrame(NULL, wxID_ANY, title) {

  // Modify the height and width values here to read and display an image with
  // different dimensions.
  //Change based on cout/argv inputs and arguements
  //This width and height should be final window size
  width = 512;
  height = 512;

  //Switch this to outWidth/outHeight or not
  unsigned char *inData = readImageData(imagePath, width, height, hue1, hue2);

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

/**Function to transfer to inData**/
unsigned char *transferInData(vector<unsigned char> red, vector<unsigned char> green, vector<unsigned char> blue, int width, int height){
  /**
   * Allocate a buffer to store the pixel values
   * The data must be allocated with malloc(), NOT with operator new. wxWidgets
   * library requires this.
   */
  unsigned char *inData = (unsigned char *)malloc(width * height * 3 * sizeof(unsigned char));
      
  for (int i = 0; i < height * width; i++) {
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
unsigned char *readImageData(string imagePath, int width, int height, int hue1, int hue2) {

  // Open the file in binary mode
  ifstream inputFile(imagePath, ios::binary);

  if (!inputFile.is_open()) {
    cerr << "Error Opening File for Reading" << endl;
    exit(1);
  }

  // Create and populate RGB buffers
  vector<char> Rbuf(width * height);
  vector<char> Gbuf(width * height);
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

  //Convert Red, Green, and Blue stream to HSV
  // Create and populate RGB buffers
  vector<float> Hbuf(width * height);
  vector<float> Sbuf(width * height);
  vector<float> Vbuf(width * height);

  for (int i=0; i < width*height; i++){
    //Hue
    float h;
    //Saturation
    float s;
    //Value (Brightness)
    float v;

    float redVal = static_cast<float>(static_cast<unsigned char>(Rbuf[i]))/255.0;
    float greenVal = static_cast<float>(static_cast<unsigned char>(Gbuf[i]))/255.0;
    float blueVal = static_cast<float>(static_cast<unsigned char>(Bbuf[i]))/255.0;

    float Cmax = max(redVal, max(greenVal, blueVal));
    float Cmin = min(redVal, min(greenVal, blueVal));

    float delta = Cmax - Cmin;

    if (Cmax == Cmin){
      h = 0;
    } else if (Cmax == redVal){
      h = (greenVal-blueVal)/delta;
    } else if (Cmax == greenVal){
      h = ((blueVal-redVal)/delta + 2.0);
    } else if (Cmax == blueVal){
      h = ((redVal-greenVal)/delta + 4.0);
    }
    h *= 60.0f;
    if ( h < 0){
      h += 360;
    }
    if (Cmax == 0){
      s = 0.0;
    } else {
      s = delta/Cmax;
    }
    v = Cmax;
    
    //cout << "Part 1 : " << h << endl;
    Hbuf[i] = h;
    Sbuf[i] = s;
    Vbuf[i] = v;
  }
  //Loop through stream and see if between hue1 and hue2
  //If within --> keep color (do nothing)
  //If not within --> make grey (saturation = 0)
  for (int i=0; i < width*height; i++){
    //MAYBE CHANGE TO INT
    int h =(Hbuf[i]); 
    if ((h < hue1 || h > hue2)){
      //cout << Hbuf[i] << endl;
      Sbuf[i] = 0;
      //Vbuf[i] *= 0.1;
    }
  }
  vector<unsigned char> newRed(width * height);
  vector<unsigned char> newGreen(width * height);
  vector<unsigned char> newBlue(width * height);

  //Turn back into RGB
  for (int i=0; i < width*height; i++){
    if (Sbuf[i] == 0){
      newRed[i] = static_cast<unsigned char> (static_cast<int>((Vbuf[i])*255));
      newGreen[i] = static_cast<unsigned char> (static_cast<int>((Vbuf[i])*255));
      newBlue[i] = static_cast<unsigned char> (static_cast<int>((Vbuf[i])*255));
    } else {
      float hp = Hbuf[i]/60;
      int hf = floor(hp);
      float f = hp - hf;

      float p = Vbuf[i]*(1 - Sbuf[i]);
      float q = Vbuf[i]*(1 - Sbuf[i] * f);
      float t = Vbuf[i]*(1 - Sbuf[i] * (1 - f));

      switch (hf) {
        case 0:
          newRed[i] = static_cast<unsigned char> (static_cast<int>((Vbuf[i])*255));
          newGreen[i] = static_cast<unsigned char> (static_cast<int>((t)*255));
          newBlue[i] = static_cast<unsigned char> (static_cast<int>((p)*255));
          break;
        case 1:
          newRed[i] = static_cast<unsigned char> (static_cast<int>((q)*255));
          newGreen[i] = static_cast<unsigned char> (static_cast<int>((Vbuf[i])*255));
          newBlue[i] = static_cast<unsigned char> (static_cast<int>((p)*255));
          break;
        case 2:
          newRed[i] = static_cast<unsigned char> (static_cast<int>((p)*255));
          newGreen[i] = static_cast<unsigned char> (static_cast<int>((Vbuf[i])*255));
          newBlue[i] = static_cast<unsigned char> (static_cast<int>((t)*255));
          break;
        case 3:
          newRed[i] = static_cast<unsigned char> (static_cast<int>((p)*255));
          newGreen[i] = static_cast<unsigned char> (static_cast<int>((q)*255));
          newBlue[i] = static_cast<unsigned char> (static_cast<int>((Vbuf[i])*255));
          break;
        case 4:
          newRed[i] = static_cast<unsigned char> (static_cast<int>((t)*255));
          newGreen[i] = static_cast<unsigned char> (static_cast<int>((p)*255));
          newBlue[i] = static_cast<unsigned char> (static_cast<int>((Vbuf[i])*255));
          break;
         default:
          newRed[i] = static_cast<unsigned char> (static_cast<int>((Vbuf[i])*255));
          newGreen[i] = static_cast<unsigned char> (static_cast<int>((p)*255));
          newBlue[i] = static_cast<unsigned char> (static_cast<int>((q)*255));
          break;
      }
    }
  }

  //Finish
  return transferInData(newRed, newGreen, newBlue, width, height);
}

wxIMPLEMENT_APP(MyApp);