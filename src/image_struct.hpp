#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

class Image
{
private:
  const cv::Mat _img;
  const std::string _img_id;
  
  std::vector<cv::KeyPoint> _kps;
  cv::Mat _des;

public:
  Image(const cv::Mat img,int nfeatures,std::string img_id="");
  void findFeatures(int nfeatures=50000);
  
  cv::Mat getImg();
  void getKpsAndDes(std::vector<cv::KeyPoint> &kps,cv::Mat &des,std::vector<cv::Rect> ROIs_features);
  std::string getID();
  bool hasFeatures();
};
