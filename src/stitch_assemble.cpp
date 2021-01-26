#include <vector>
#include <opencv2/opencv.hpp>

void stitchImages(const std::vector<cv::Mat> &imgs,
                     const std::vector<cv::Mat> &homo_mats)
{
  //auto cumul_homo_mats = accumulateHomoMats(homo_mats);

  cv::Mat stitch;
  cv::Size stitch_size(1844,3000);

  for (int i = 0; i < imgs.size()-1; i++)
  {
    cv::warpPerspective(imgs[i+1],stitch,homo_mats[i+1],stitch_size);
    cv::imwrite("/home/yquek/Downloads/"+std::to_string(i)+"a.jpg",stitch);
    cv::Mat ROI = stitch.rowRange(0,imgs[i].rows);
    imgs[i].copyTo(ROI);
    cv::imwrite("/home/yquek/Downloads/"+std::to_string(i)+"b.jpg",stitch);
  }
}

std::vector<cv::Mat> accumulateHomoMats(const std::vector<cv::Mat> &homo_mats)
{
  auto cumul_homo_mat = cv::Mat::eye(3,3,CV_32F);
  std::vector<cv::Mat> cumul_homo_mats;
  for (const auto &homo_mat : homo_mats)
  {
    cumul_homo_mat *= homo_mat;
    cumul_homo_mats.push_back(cumul_homo_mat);
  }
  return cumul_homo_mats;
}
