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
vector<vector<double>> cosTableU;
vector<vector<double>> cosTableV;
vector<vector<double>> red2D;
vector<vector<double>> green2D;
vector<vector<double>> blue2D;
vector<vector<double>> DCTRed(512, vector<double>(512));
vector<vector<double>> DCTGreen(512, vector<double>(512));
vector<vector<double>> DCTBlue(512, vector<double>(512));
vector<vector<double>> DWTRed(512, vector<double>(512));
vector<vector<double>> DWTGreen(512, vector<double>(512));
vector<vector<double>> DWTBlue(512, vector<double>(512));

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
  MyFrame(const wxString &title, string imagePath, int n, bool isDCT, bool DWTB);
  void updateData(string imagePath, int width, int height, int n, bool isDCT, bool DWTB);

 private:
  void OnPaint(wxPaintEvent &event);
  wxImage inImage;
  wxScrolledWindow *scrolledWindow;
  int width;
  int height;
};

/** Utility function to read image data */
unsigned char *readImageData(string imagePath, int width, int height, int n, bool isDCT, bool DWTB);
/**inData function**/
unsigned char* transferInData(vector<unsigned char> red, vector<unsigned char> green, vector<unsigned char> blue, int outWidth, int outHeight);
//Create 2D vector of R/G/B stream
vector<vector<double>> to2D(vector<char> buf, int height, int width);
//2D output to 1D stream
vector<unsigned char> to1D(vector<vector<double>> output2D, int height, int width);
//Function for CosineTables
vector<vector<double>> outputCosineTableV(int sizeY, int sizeX);
vector<vector<double>> outputCosineTableU(int sizeY, int sizeX);
vector<vector<double>> outputDCTBlock(vector<vector<double>> ogBlock, int offsetX, int offsetY, vector<vector<double>> cosTableU, vector<vector<double>> cosTableV);
vector<vector<double>> outputDWT(vector<vector<double>>block, int height, int width);

/** Definitions */

/**
 * Init method for the app.
 * Here we process the command line arguments and
 * instantiate the frame.
 */
bool MyApp::OnInit() {
  wxInitAllImageHandlers();
  cout << "Number of command line arguments: " << wxApp::argc << endl;
  if (wxApp::argc != 3) {
    cerr << "The executable should be invoked with exactly one filepath "
            "argument. Example ./MyImageApplication '../../Lena_512_512.rgb' n"
         << endl;
    exit(1);
  }
  cout << "First argument: " << wxApp::argv[0] << endl;
  cout << "Second argument: " << wxApp::argv[1] << endl;
  string imagePath = wxApp::argv[1].ToStdString();
  cout << "Third argument: " << wxApp::argv[2] << endl;
  int n = wxAtoi(argv[2]);
  string title;
  string title2;
  //Create Cosine Table
  cosTableU = outputCosineTableU(8,8);
  cosTableV = outputCosineTableV(8,8);
  cout << "Finished cosine table creation" << endl;

  //If n > 0
  if (n > 0){
    //DCT = 1 
    title = "DCT with n = " + to_string(n);
    MyFrame *frame = new MyFrame(title, imagePath, n, true, false);
    frame->SetPosition(wxPoint(25,100));
    frame->Show(true);

    //DWT = 0
    title = "DWT with n = " + to_string(n);
    MyFrame *frame2 = new MyFrame(title, imagePath, n, false, false);
    frame2->SetPosition(wxPoint(550,100));
    frame2->Show(true);
  } else if (n == -1){
    //DCT
    title = "DCT Progressive (n == -1) n == 4096";
    MyFrame *DCTProgFrame = new MyFrame(title, imagePath, 4096, true, false);
    DCTProgFrame->SetPosition(wxPoint(25,100));
    DCTProgFrame->Show(true);
    //DWT
    title2 = "DWT Progressive (n == -1) k == 0";
    MyFrame *DWTProgFrame = new MyFrame(title2, imagePath, 1, false, false);
    DWTProgFrame->SetPosition(wxPoint(550,100));
    DWTProgFrame->Show(true);
    DWTProgFrame->Update();
    //Loop
    int mult = 2;
    int coeff = 4096;
    int k = 1; 
    int kNum;

    while (mult <= 64){
      DCTProgFrame->updateData(imagePath, 512, 512, coeff*mult, true, false);
      DCTProgFrame->SetLabel(wxT("DCT Progressive (n == -1) n == " + to_string(coeff*mult)));
      mult++;
      if (k < 10){
        kNum = pow(2,k) * pow(2,k);
        DWTProgFrame->updateData(imagePath, 512, 512, kNum, false, false);
        DWTProgFrame->SetLabel(wxT("DWT Progressive (n == -1) k == " + to_string(k)));
        k++;
      }
      wxYield(); 
    }
  } else if (n == -2){
    //DCT
    title = "DCT Progressive (n == -2) n == 4096";
    MyFrame *DCTProgFrame = new MyFrame(title, imagePath, 4096, true, false);
    DCTProgFrame->SetPosition(wxPoint(25,100));
    DCTProgFrame->Show(true);
    //DWT
    title2 = "DWT Progressive (n == -2) n == 4096";
    MyFrame *DWTProgFrame = new MyFrame(title2, imagePath, 4096, false, true);
    DWTProgFrame->SetPosition(wxPoint(550,100));
    DWTProgFrame->Show(true);
    DWTProgFrame->Update();
    //Loop
    int mult = 2;
    int coeff = 4096;
    while (mult <= 64){
      DCTProgFrame->updateData(imagePath, 512, 512, coeff*mult, true, false);
      DCTProgFrame->SetLabel(wxT("DCT Progressive (n == -2) n == " + to_string(coeff*mult)));

      DWTProgFrame->updateData(imagePath, 512, 512, coeff*mult, false, true);
      DWTProgFrame->SetLabel(wxT("DWT Progressive (n == -2) n == " + to_string(coeff*mult)));
      mult++;
      wxYield(); 
    }
  }
  // return true to continue, false to exit the application
  return true;
}

/**
 * Constructor for the MyFrame class.
 * Here we read the pixel data from the file and set up the scrollable window.
 */
MyFrame::MyFrame(const wxString &title, string imagePath, int n, bool isDCT, bool DWTB)
    : wxFrame(NULL, wxID_ANY, title) {

  // Modify the height and width values here to read and display an image with
  // different dimensions.
  //Change based on cout/argv inputs and arguements
  //This width and height should be final window size
  width = 512;
  height = 512;
  
  // Set up the scrolled window as a child of this frame
  scrolledWindow = new wxScrolledWindow(this, wxID_ANY);
  scrolledWindow->SetScrollbars(10, 10, width, height);
  scrolledWindow->SetVirtualSize(width, height);

  //Data every loop needs
  #pragma region
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
  red2D = to2D(Rbuf, height, width);
  green2D = to2D(Gbuf, height, width);
  blue2D = to2D(Bbuf, height, width);

  //Part 1 - Encode it
  if (isDCT){
      // Create DCT Table
      for (int i = 0; i < height; i += 8) {
          for (int j = 0; j < width; j += 8) {
              vector<vector<double>> chunkRed =outputDCTBlock(red2D, i, j, cosTableU, cosTableV);
              vector<vector<double>> chunkGreen =outputDCTBlock(green2D, i, j, cosTableU, cosTableV);
              vector<vector<double>> chunkBlue =outputDCTBlock(blue2D, i, j, cosTableU, cosTableV);
              for (int y = 0; y < 8; y++) {
                  for (int x = 0; x < 8; x++) {
                      DCTRed[i + y][j + x] = chunkRed[y][x];
                      DCTGreen[i + y][j + x] = chunkGreen[y][x];
                      DCTBlue[i + y][j + x] = chunkBlue[y][x];
                  }
              }
          }
      }
      cout << "Finished DCT Encoding" << endl;
  } else {
      // DWT
      // Part 1 - Encode it
      DWTRed = outputDWT(red2D, height, width);
      DWTGreen = outputDWT(green2D, height, width);
      DWTBlue = outputDWT(blue2D, height, width);
      cout << "Finished DWT Encoding" << endl;
  }
  #pragma endregion

  unsigned char *inData = readImageData(imagePath, width, height, n, isDCT, DWTB);

  // the last argument is static_data, if it is false, after this call the
  // pointer to the data is owned by the wxImage object, which will be
  // responsible for deleting it. So this means that you should not delete
  // the data yourself.
  inImage.SetData(inData, width, height, false);
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
void MyFrame::updateData(string imagePath, int width, int height, int n, bool isDCT, bool DWTB){
  scrolledWindow->Update();
  unsigned char *inData = readImageData(imagePath, width, height, n, isDCT, DWTB);
  inImage.SetData(inData, width, height, false);
  scrolledWindow->Refresh();
  scrolledWindow->Update();
}

/** Function to convert 1D stream of color to 2D vector and normalize rgb value*/
vector<vector<double>> to2D(vector<char> buf, int height, int width){
  vector<vector<double>> image2D(height, vector<double>(width));
  for (int i = 0; i < height; i++){
    for (int j = 0; j < width; j++){
      int pixelIndex = (i * width + j);
      image2D[i][j] = static_cast<double>(static_cast<unsigned char> (buf[pixelIndex]));
    }
  }
  return image2D;
}
/** Function to turn 2D back into readable 1D stream and unnormalize rgb value**/
vector<unsigned char> to1D(vector<vector<double>> output2D, int height, int width){
  vector<unsigned char> buf(width * height);
  int index = 0;
  double test;
  unsigned char test2;
  for (int i = 0; i < height; i++){
    for (int j = 0; j < width; j++){
      buf[index] = static_cast<unsigned char>((output2D[i][j]));
      index++;
    }
  }
  return buf;
}
/**Function to output 8x8 DCT block**/
vector<vector<double>> outputDCTBlock(vector<vector<double>> ogBlock, int offsetX, int offsetY, vector<vector<double>> cosTableU, vector<vector<double>> cosTableV) {
    vector<vector<double>> block(8, vector<double>(8));
    // Do the equation
    double sum;
    double CU;
    double CV;
    for (int v = 0; v < 8; v++) {
        for (int u = 0; u < 8; u++) {
          sum = 0.0;
            if (u == 0) {
                CU = 1.0 / (sqrt(2.0));
            } else {
                CU = 1.0;
            }
            if (v == 0) {
                CV = 1.0 / (sqrt(2.0));
            } else {
                CV = 1.0;
            }
            for (int y = 0; y < 8; y++) {
                for (int x = 0; x < 8; x++) {
                    sum += ogBlock[y + offsetY][x + offsetX] * cosTableU[u][x] * cosTableV[v][y];
                }
            }
            block[v][u] = sum * 0.25 * CU * CV;
        }
    }
    return block;
}
/**Function to output 8x8 IDCT block**/
vector<vector<double>> outputIDCTBlock(vector<vector<double>> ogBlock, int offsetX, int offsetY, vector<vector<double>> cosTableU, vector<vector<double>> cosTableV) {
    vector<vector<double>> block(8, vector<double>(8));
    // Do the equationint
    double sum;
    double CU;
    double CV;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
          sum = 0.0;
            for (int v = 0; v < 8; v++) {
                for (int u = 0; u < 8; u++) {
                    if (u == 0) {
                        CU = 1.0 / (sqrt(2.0));
                    } else {
                        CU = 1.0;
                    }
                    if (v == 0) {
                        CV = 1.0 / (sqrt(2.0));
                    } else {
                        CV = 1.0;
                    }
                    sum += CU * CV * ogBlock[v + offsetY][u + offsetX] * cosTableU[u][x] * cosTableV[v][y];
                }
            }
            block[y][x] = clamp((sum * 0.25), 0.0, 255.0);
        }
    }
    return block;
}

/**Cosine Table Function**/
vector<vector<double>> outputCosineTableV(int sizeY, int sizeX){
  vector<vector<double>> table(sizeY, vector<double>(sizeX));
  for (int v = 0; v < sizeY; v++) {
      for (int y = 0; y < sizeY; y++) {
          table[v][y] = cos((2.0 * y + 1.0) * v * M_PI / 16.0);
        }
      }
  return table;
}
vector<vector<double>> outputCosineTableU(int sizeY, int sizeX){
  vector<vector<double>> table(sizeY, vector<double>(sizeX));
    for (int u = 0; u < sizeX; u++) {
        for (int x = 0; x < sizeX; x++) {
          table[u][x] = cos((2.0 * x + 1.0) * u * M_PI / 16.0);
        }
      }
  return table;
}

/**Function to calculate DWT**/
vector<vector<double>> outputDWT(vector<vector<double>>block, int height, int width){
  vector<vector<double>> tempBlock(height, vector<double>(width));
  vector<vector<double>> block1 = block;
  while(height > 1 && width > 1){ 
  //Row pass
  for (int j = 0; j < height; j++){
    for (int i = 0; i < width/2; i++){
      //Avg (Low Pass)
      tempBlock[j][i] = (block1[j][2*i] + block1[j][2*i+1])/2.0;
      //Diff (High Pass)
      tempBlock[j][i + width/2] = (block1[j][2*i] - block1[j][2*i+1])/2.0;
    }
  }
  //Column Pass
  for (int j = 0; j < width; j++){
    for (int i = 0; i < height/2; i++){
      //Avg (Low Pass)
      block1[i][j] = (tempBlock[2*i][j] + tempBlock[2*i+1][j])/2.0;
      //Diff (High Pass)
      block1[i +  height/2][j] = (tempBlock[2*i][j] - tempBlock[2*i+1][j])/2.0;
    }
  }

  height /= 2;
  width /= 2;
  }
  return block1;
}
/**Function to calculate IDWT**/
vector<vector<double>> outputIDWT(vector<vector<double>>block, int height, int width){
  vector<vector<double>> tempBlock(height, vector<double>(width));
  vector<vector<double>> block1 = block;
  int row = 1;
  int col = 1; 
  while(row <= height && col <= width){
    if (row == 1 && col == 1){
      row*=2;
      col*=2;
      continue;
    }
    //Column Pass
    for (int j = 0; j < col; j++){
      for (int i = 0; i < row/2; i++){
        //Avg (Low Pass)
        tempBlock[2*i][j] = (block1[i][j] + block1[i + row/2][j]);
        //Diff (High Pass)
        tempBlock[2*i + 1][j] = (block1[i][j] - block1[i + row/2][j]);
      }
    }
    //Row pass
    for (int j = 0; j < row; j++){
      for (int i = 0; i < col/2; i++){
        //Avg (Low Pass)
        block1[j][2*i] = (tempBlock[j][i] + tempBlock[j][i+col/2]);
        //Diff (High Pass)
        block1[j][2*i + 1] = (tempBlock[j][i] - tempBlock[j][i+col/2]);
      }
    }
  row*=2;
  col*=2;
  }

  //Keep between 0 - 255
  for (int y = 0; y < height; y++){
    for (int x = 0; x < width; x++){
      double temp = block1[y][x];
      block1[y][x] = clamp(temp, 0.0, 255.0);
    }
  }
  return block1;
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
unsigned char *readImageData(string imagePath, int width, int height, int n, bool isDCT, bool DWTB) {
  if (isDCT && n >0){
  vector<vector<double>> DCTRedCopy = DCTRed;
  vector<vector<double>> DCTGreenCopy = DCTGreen;
  vector<vector<double>> DCTBlueCopy = DCTBlue;
  //DCT
  //Part 2 - Decode it
  int m = static_cast<int> (round(n/4096));
  int mConst = m;
  int row = 0;
  int col = 0;
  bool up = true;

  for (int i = 0; i < height; i += 8) {
      for (int j = 0; j < width; j += 8) {
        row = 0;
        col = 0;
        up = true;
        m = mConst;
          while (row < 8 && col < 8) {
              if (m <= 0) {
                  DCTRedCopy[row + i][col + j] = 0;
                  DCTGreenCopy[row + i][col + j] = 0;
                  DCTBlueCopy[row + i][col + j] = 0;
              }
              if (up) {
                  if (row == 0 || col == 7) {
                      up = false;
                      if (col == 7) {
                          row += 1;
                      } else {
                          col += 1;
                      }
                  } else {
                      row -= 1;
                      col += 1;
                  }
              } else {
                  if (row == 7 || col == 0) {
                      up = true;
                      if (row == 7) {
                          col += 1;
                      } else {
                          row += 1;
                      }
                  } else {
                      row += 1;
                      col -= 1;
                  }
              }
              m -= 1;
          }
      }
  }
  cout << "Finished Zig Zag" << endl;
  //IDCT
  vector<vector<double>> IDCTRed(height, vector<double>(width));
  vector<vector<double>> IDCTGreen(height, vector<double>(width));
  vector<vector<double>> IDCTBlue(height, vector<double>(width));
  for (int i = 0; i < height; i+=8){
    for (int j = 0; j < width; j+=8){
      vector<vector<double>> chunkRed= outputIDCTBlock(DCTRedCopy, i, j, cosTableU, cosTableV);
      vector<vector<double>> chunkGreen= outputIDCTBlock(DCTGreenCopy, i, j, cosTableU, cosTableV);
      vector<vector<double>> chunkBlue= outputIDCTBlock(DCTBlueCopy, i, j, cosTableU, cosTableV);

      for (int y = 0; y < 8; y++){
        for (int x = 0; x < 8; x++){
          IDCTRed[i + y][j + x] = chunkRed[y][x];
          IDCTGreen[i + y][j + x] = chunkGreen[y][x];
          IDCTBlue[i + y][j + x] = chunkBlue[y][x];
        }
      }
    }
  }
  cout << "Finished IDCT Decoding" << endl;

  vector<unsigned char> newRed(width * height);
  vector<unsigned char> newGreen(width * height);
  vector<unsigned char> newBlue(width * height);

  newRed = to1D(IDCTRed, height, width);
  newGreen = to1D(IDCTGreen, height, width);
  newBlue = to1D(IDCTBlue, height, width);
  //Finish
  cout << "DONE DCT WITH n = " + to_string(n) << endl;
  return transferInData(newRed, newGreen, newBlue, width, height);

  } else if (!isDCT && n > 0){
  //DWT
  vector<vector<double>> DWTRedCopy(height, vector<double>(width, 0.0));
  vector<vector<double>> DWTGreenCopy(height, vector<double>(width, 0.0));
  vector<vector<double>> DWTBlueCopy(height, vector<double>(width, 0.0));
  if (!DWTB){
  //Part 2 - Decode it
  int coeffDim = sqrt(n);
  for (int i = 0; i < coeffDim; i++){
    for (int j = 0; j < coeffDim; j++){
        DWTRedCopy[i][j] = DWTRed[i][j];
        DWTGreenCopy[i][j] = DWTGreen[i][j];
        DWTBlueCopy[i][j] = DWTBlue[i][j];
    } 
  }
  }else {
    /*
    vector<vector<int>> coeffOrder {
      {0,0},{0,1},{1,0},{1,1},{0,2},{0,3},{1,2},{1,3},{2,0},{2,1},{3,0},{3,1},{2,2},{2,3},{3,2},{3,3},
      {1,5},{1,6},{2,5},{2,6},{0,4},{0,5},{0,6},{0,7},{1,4},{1,7},{2,4},{2,7},{3,4},{3,5},{3,6},{3,7},
      {5,1},{5,2},{6,1},{6,2},{4,0},{4,1},{4,2},{4,3},{5,0},{5,3},{6,0},{6,3},{7,0},{7,1},{7,2},{7,3},
      {5,5},{5,6},{6,5},{6,6},{4,4},{4,5},{4,6},{4,7},{5,4},{5,7},{6,4},{6,7},{7,4},{7,5},{7,6},{7,7}   
    };*/
    vector<vector<int>> coeffOrder {
      {0,0},{0,1},{1,0},{1,1},{0,2},{0,3},{1,2},{1,3},{2,0},{2,1},{3,0},{3,1},{2,2},{2,3},{3,2},{3,3},
      {0,4},{0,5},{0,6},{0,7},{1,4},{1,5},{1,6},{1,7},{2,4},{2,5},{2,6},{2,7},{3,4},{3,5},{3,6},{3,7},
      {4,0},{4,1},{4,2},{4,3},{5,0},{5,1},{5,2},{5,3},{6,0},{6,1},{6,2},{6,3},{7,0},{7,1},{7,2},{7,3},
      {4,4},{4,5},{4,6},{4,7},{5,4},{5,5},{5,6},{5,7},{6,4},{6,5},{6,6},{6,7},{7,4},{7,5},{7,6},{7,7}
    };
    int m = static_cast<int> (round(n/4096));
    int block = 0; 
    int blockSize = 64;
    int row;
    int col;
    while (block < m){
      row = coeffOrder[block][0];
      col = coeffOrder[block][1];
      for (int i = 0; i < blockSize; i++){
        for (int j = 0; j < blockSize; j++){
          DWTRedCopy[row*64 + i][col*64 + j] = DWTRed[row*64 + i][col*64 + j];
          DWTGreenCopy[row*64 + i][col*64 + j] = DWTGreen[row*64 + i][col*64 + j];
          DWTBlueCopy[row*64 + i][col*64 + j] = DWTBlue[row*64 + i][col*64 + j];
        }
      }
      block++;
    }
  }
  cout << "Finished Coefficient Zeroing"<< endl;
  //IDWT
  vector<vector<double>> IDWTRed(height, vector<double>(width));
  vector<vector<double>> IDWTGreen(height, vector<double>(width));
  vector<vector<double>> IDWTBlue(height, vector<double>(width));

  IDWTRed = outputIDWT(DWTRedCopy, height, width);
  IDWTGreen = outputIDWT(DWTGreenCopy, height, width);
  IDWTBlue = outputIDWT(DWTBlueCopy, height, width);
  cout << "Finished IDWT Decoding"<< endl;

  vector<unsigned char> newRed(width * height);
  vector<unsigned char> newGreen(width * height);
  vector<unsigned char> newBlue(width * height);

  newRed = to1D(IDWTRed, height, width);
  newGreen = to1D(IDWTGreen, height, width);
  newBlue = to1D(IDWTBlue, height, width);
  
  
  //Finish
  cout << "DONE DWT WITH n = " + to_string(n) << endl;
  return transferInData(newRed, newGreen, newBlue, width, height);
  }
  cout << "you shouldn't be here!" << endl;
  return NULL;
}

wxIMPLEMENT_APP(MyApp);