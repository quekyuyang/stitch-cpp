#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include "video_sync.hpp"
#include "params_reader.hpp"

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
  std::map<std::string,cv::Mat> imgs;
  for (const auto &ID : IDs)
    imgs[ID] = frameset[ID];

  Network network(jpath);

  return 0;
}
