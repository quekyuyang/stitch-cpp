#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
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

  std::string vid_group_path(argv[1]);
  VideoSynchronizer vid_sync(vid_group_path);
  std::string jpath(argv[2]);
  std::vector<std::string> IDs(argv+3,argv+argc);

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

  cv::Size resolution(1920,1080);
  int codec = cv::VideoWriter::fourcc('m','p','4','v');
  cv::VideoWriter vid_stitch(vid_group_path+"/stitched/video.mp4",codec,30,resolution);
  while (true)
  {
    auto frameset = vid_sync.getFrames();
    std::vector<cv::Mat> imgs;
    for (const auto &ID : IDs)
      imgs.push_back(frameset[ID]);

    auto stitched = stitchImages(imgs,params_means);
    cv::rotate(stitched,stitched,cv::ROTATE_90_COUNTERCLOCKWISE);
    cv::resize(stitched,stitched,resolution);
    vid_stitch.write(stitched);
    cv::imshow("Stitched",stitched);
    char user_input = cv::waitKey(1);
    if (user_input == 27)
    {
      std::cout << "Interrupted by user" << std::endl;
      break;
    }
  }

  return 0;
}
