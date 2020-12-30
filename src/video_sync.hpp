#include <string>
#include <map>
#include <vector>
#include <opencv2/opencv.hpp>

class Interval
{
public:
  Interval();
  void setBegin(std::vector<int>::size_type);
  void setEnd(std::vector<int>::size_type);
  std::vector<int>::size_type getBegin() const;
  std::vector<int>::size_type getEnd() const;
  
private:
  std::vector<int>::size_type _begin;
  std::vector<int>::size_type _end;
};

class Stream
{
  public:
    Stream(cv::VideoCapture vid, std::vector<int> trig_nums, Interval interval, std::string ID);
    ~Stream();
    void getFrame(int query_trig_n, cv::Mat &frame);
    std::string getID();
  private:
    cv::VideoCapture _vid;
    std::vector<int> _trig_nums;
    Interval _interval;
    std::string _ID;
    cv::Mat _frame_last;
};

class FrameSet
{
public:
  FrameSet(std::map<std::string, cv::Mat> imgs, int trig_n);
  cv::Mat operator[](std::string ID);
  int getTrigN() const;
  void checkForEmptyFrames();

private:
  std::map<std::string, cv::Mat> _imgs;
  const int _trig_n;
};

class VideoSynchronizer
{
public:
  VideoSynchronizer(std::string dirpath,int skip_init_frames=0);
  FrameSet getFrames();
  
private:
  std::vector<Stream> _streams;
  int _trig_n;
};

std::vector<int> readTrigN(std::string filepath);

std::map<std::string, Interval> getFrameIntervals(const std::map<std::string, std::vector<int>> trig_nums_streams);

std::map<std::string,std::vector<int>::size_type> getTrigNIdxs(const std::map<std::string,std::vector<int>> &trig_nums_streams, int trig_n);
