#include <vector>
#include <opencv2/opencv.hpp>

void stitchImages(const std::vector<cv::Mat> &imgs,
                     const std::vector<cv::Mat> &homo_mats);

std::vector<cv::Mat> accumulateHomoMats(const std::vector<cv::Mat> &homo_mats);
