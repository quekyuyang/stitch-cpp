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
  std::vector<cv::KeyPoint> _kps_histequal;
  cv::Mat _des_histequal;

public:
  Image(const cv::Mat img = cv::Mat(),int nfeatures = 0,std::string img_id = "");
  void findFeatures(int nfeatures,const bool histequal);

  cv::Mat getImg();
  void getKpsAndDes(const int nfeatures,const bool histequal,
    std::vector<cv::KeyPoint> &kps_filter,cv::Mat &des_filter,
    std::vector<cv::Rect> ROIs_features);
  std::string getID();
  bool hasFeatures();
};



cv::Mat equalizeHist(const cv::Mat &img);
