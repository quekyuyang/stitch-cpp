#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include "video_sync.hpp"

VideoSynchronizer::VideoSynchronizer(std::string dirpath,int skip_init_frames)
{
  std::map<std::string, std::vector<int>> trig_nums_streams;
  std::string meta_dirpath = dirpath + "/meta";
  for (auto &entry : std::filesystem::directory_iterator(meta_dirpath))
  {
    std::string filename(entry.path().filename());
    std::string stream_ID = filename.substr(0,6);
    trig_nums_streams[stream_ID] = readTrigN(entry.path());
  }

  auto intervals = getFrameIntervals(trig_nums_streams);

  std::string vid_dirpath = dirpath + "/videos";
  for (auto &entry : std::filesystem::directory_iterator(vid_dirpath))
  {
    std::string filename(entry.path().filename());
    std::string stream_ID = filename.substr(0,6);
    _streams.push_back(Stream(cv::VideoCapture(entry.path()),trig_nums_streams[stream_ID],intervals[stream_ID],stream_ID));
  }

  std::string stream_ID = trig_nums_streams.begin()->first;
  _trig_n = trig_nums_streams[stream_ID][intervals[stream_ID].getBegin()];
  _trig_n--;
}

FrameSet VideoSynchronizer::getFrames()
{
  _trig_n += 1;
  std::cout << "Trigger Number: " << _trig_n << std::endl;

  std::map<std::string, cv::Mat> imgs;
  for (int i=0; i<_streams.size();i++)
  {
    std::string stream_ID(_streams[i].getID());
    cv::Mat img;
    _streams[i].getFrame(_trig_n,imgs[stream_ID]);
  }

  FrameSet frameset(imgs,_trig_n);
  return frameset;
}

FrameSet::FrameSet(std::map<std::string, cv::Mat> imgs, int trig_n)
  : _imgs(imgs), _trig_n(trig_n)
{
  checkForEmptyFrames();
}

cv::Mat FrameSet::operator[](std::string ID) {return _imgs[ID];}
int FrameSet::getTrigN() const {return _trig_n;}

void FrameSet::checkForEmptyFrames() const
{
  int n_empty = 0;
  for (const auto &[ID,img] : _imgs)
  {
    if (img.empty())
      n_empty++;
  }

  if (n_empty == 0)
    return;
  else if (n_empty == _imgs.size())
    throw std::runtime_error("Finished processing videos");
  else if (n_empty > 0 && n_empty < _imgs.size())
    throw std::runtime_error("Missing frames");
}

Stream::Stream(cv::VideoCapture vid, std::vector<int> trig_nums, Interval interval, std::string ID)
  : _vid(vid), _trig_nums(trig_nums), _interval(interval), _ID(ID)
{
  _vid.set(cv::CAP_PROP_POS_FRAMES,_interval.getBegin());
}

Stream::~Stream() {_vid.release();}
std::string Stream::getID() {return _ID;}

void Stream::getFrame(int query_trig_n, cv::Mat &frame)
{
  auto vid_pos = _vid.get(cv::CAP_PROP_POS_FRAMES);
  if (vid_pos > _interval.getEnd())
  {
    std::cout << "Reached end of stream " << _ID << std::endl;
    return;
  }

  if (_trig_nums[vid_pos] == query_trig_n)
  {
    if (!_vid.read(frame))
    {
      frame = cv::Mat();
      return;
    }

    _frame_last = frame;
  }
  else if (!_frame_last.empty())
    frame = _frame_last;
  else
    throw std::runtime_error("No last frame for stream " + _ID);
}

Interval::Interval()
{}

void Interval::setBegin(std::vector<int>::size_type begin) {_begin = begin;}
void Interval::setEnd(std::vector<int>::size_type end) {_end = end;}
std::vector<int>::size_type Interval::getBegin() const {return _begin;}
std::vector<int>::size_type Interval::getEnd() const {return _end;}

std::vector<int> readTrigN(std::string filepath)
{
  std::vector<int> trig_nums;
  std::ifstream filestream(filepath);
  std::string line;

  while (std::getline(filestream,line))
  {
    std::istringstream linestream(line);
    int trig_n;
    linestream >> trig_n;

    trig_nums.push_back(trig_n);
  }

  return trig_nums;
}

std::map<std::string, Interval> getFrameIntervals(const std::map<std::string, std::vector<int>> trig_nums_streams)
{
  std::map<std::string, Interval> intervals;

  // Find interval beginnings
  std::vector<int> first_trig_nums;
  for (const auto &[ID,trig_nums] : trig_nums_streams)
    first_trig_nums.push_back(trig_nums.front());
  int latest_first_trig_n = *std::max_element(first_trig_nums.begin(),first_trig_nums.end());

  std::map<std::string,std::vector<int>::size_type> interval_start_idxs;
  while (interval_start_idxs.empty())
  {
    interval_start_idxs = getTrigNIdxs(trig_nums_streams,latest_first_trig_n);
    latest_first_trig_n++;
  }

  for (const auto [ID,interval_start_idx] : interval_start_idxs)
  {
    intervals[ID]; // Create map entry with default initialized Interval
    intervals[ID].setBegin(interval_start_idx);
  }

  // Find interval ends
  std::vector<int> last_trig_nums;
  for (const auto &[ID,trig_nums] : trig_nums_streams)
    last_trig_nums.push_back(trig_nums.back());
  int earliest_last_trig_n = *std::min_element(last_trig_nums.begin(),last_trig_nums.end());

  std::map<std::string,std::vector<int>::size_type> interval_end_idxs;
  while (interval_end_idxs.empty())
  {
    interval_end_idxs = getTrigNIdxs(trig_nums_streams,earliest_last_trig_n);
    earliest_last_trig_n--;
  }

  for (const auto [ID,interval_end_idx] : interval_end_idxs)
    intervals[ID].setEnd(interval_end_idx);

  return intervals;
}

std::map<std::string,std::vector<int>::size_type> getTrigNIdxs(const std::map<std::string,std::vector<int>> &trig_nums_streams, int trig_n)
{
  std::map<std::string,std::vector<int>::size_type> trig_n_idxs;

  for (const auto &[ID,trig_nums] : trig_nums_streams)
  {
    auto it_trig_n = std::find(trig_nums.begin(),trig_nums.end(),trig_n);
    if (it_trig_n != trig_nums.end())
      trig_n_idxs[ID] = it_trig_n - trig_nums.begin();
    else // if any trigger number list does not contain queried trigger number, return empty map
    {
      trig_n_idxs.clear();
      break;
    }
  }
  return trig_n_idxs;
}
