#include <vector>
#include <opencv2/opencv.hpp>

cv::Mat stitchImages(const std::vector<cv::Mat> &imgs,
                  const std::vector<cv::Mat> &homo_mats);

cv::Mat getTransformedVerts(const cv::Size &img_size,const cv::Mat &homo_mat);

cv::Mat getVertsBoundaries(const std::vector<cv::Mat> verts_sets);

std::vector<cv::Mat> accumulateHomoMats(const std::vector<cv::Mat> &homo_mats);

std::vector<cv::Point> C2_to_points(const cv::Mat C2_mat);

cv::Mat makeTransMat(const int x_trans,const int y_trans);

void addTransMats(std::vector<cv::Mat> &homo_mats,
                  const int x_trans,const int y_trans);

void addTransVerts(std::vector<cv::Mat> &verts_sets,
                   const int x_trans,const int y_trans);
