#include <vector>
#include <opencv2/opencv.hpp>

void stitchImages(const std::vector<cv::Mat> &imgs,
                  const std::vector<cv::Mat> &homo_mats);

cv::Mat getTransformedVerts(const cv::Size &img_size,const cv::Mat &homo_mat);

cv::Mat getVertsBoundaries(const std::vector<cv::Mat> verts_sets);

std::vector<cv::Mat> accumulateHomoMats(const std::vector<cv::Mat> &homo_mats);
