#include <vector>
#include <opencv2/opencv.hpp>

cv::Mat findStitchParams(std::vector<cv::Mat> &imgs,const int nfeatures,
                         const int max_error_inliers,const int min_n_inliers);

void getMatches(cv::Mat des1,cv::Mat des2,std::vector<cv::DMatch> &matches);

cv::Mat equalizeHist(cv::Mat &img);
