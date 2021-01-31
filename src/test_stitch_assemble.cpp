#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <opencv2/highgui.hpp>
#include "video_sync.hpp"
#include "params_reader.hpp"
#include "stitch_assemble.hpp"

int main(int argc, char** argv)
{
  if ( argc < 4 )
  {
    std::cout << "usage: ./sync_videos <video group path> <parameters json path> <video ID 1> <video ID 2> ..." << std::endl;
    return -1;
  }

  VideoSynchronizer vid_sync(argv[1]);
  std::string jpath(argv[2]);
  std::vector<std::string> IDs(argv+3,argv+argc);

  auto frameset = vid_sync.getFrames();
  std::vector<cv::Mat> imgs;
  for (const auto &ID : IDs)
    imgs.push_back(frameset[ID]);

  auto stitch_config = getParams(jpath);
  std::vector<cv::Mat> params_means;
  for (const auto &[ID,params_log] : stitch_config)
  {
    cv::Mat params_log_mat;
    for (const auto &params : params_log)
      params_log_mat.push_back(params);

    params_log_mat = params_log_mat.reshape(1,params_log_mat.total()/9);
    cv::Mat params_mean;
    cv::reduce(params_log_mat,params_mean,0,cv::REDUCE_AVG);
    params_means.push_back(params_mean.reshape(1,3));
  }

  auto stitched = stitchImages(imgs,params_means);
  cv::resize(stitched,stitched,cv::Size(),0.25,0.25);
  cv::imshow("Stitched",stitched);
  cv::waitKey(0);

  return 0;
}
