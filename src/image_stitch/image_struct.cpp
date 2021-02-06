#include <string>
#include <vector>
#include <memory>
#include <opencv2/opencv.hpp>
#include "image_struct.hpp"

Image::Image(const cv::Mat img, int nfeatures, std::string img_id)
            :_img(img), _img_id(img_id)
{}

void Image::findFeatures(int nfeatures,const bool histequal)
{
  cv::Mat img;
  cv::Ptr<cv::ORB> orb = cv::ORB::create(nfeatures);
  if (histequal && _kps_histequal.empty())
  {
    img = equalizeHist(_img);
    orb->detectAndCompute(img,cv::noArray(),_kps_histequal,_des_histequal,false);
  }
  else if (!histequal && _kps.empty())
  {
    img = _img;
    orb->detectAndCompute(img,cv::noArray(),_kps,_des,false);
  }
  else
    return;
}

cv::Mat Image::getImg()
{
  return _img;
}

void Image::getKpsAndDes(std::vector<cv::KeyPoint> &kps_filter,cv::Mat &des_filter,
                         std::vector<cv::Rect> ROIs_features,
                         const bool histequal)
{
  if (histequal)
  {
    if (_kps_histequal.empty())
      return;

    std::vector<unsigned int> des_row_mask;
    auto n_kps = _kps_histequal.size();
    for (int i = 0; i < n_kps; i++)
    {
      bool is_in_BB = false;
      for (auto ROI_features : ROIs_features)
        is_in_BB = is_in_BB || ROI_features.contains(_kps_histequal[i].pt);

      if (is_in_BB)
      {
        kps_filter.push_back(_kps_histequal[i]);
        des_row_mask.push_back(i);
      }
    }

    des_filter.create(des_row_mask.size(),_des_histequal.cols,_des_histequal.type());
    auto n_des = des_row_mask.size();
    for (int i = 0; i < n_des; i++)
      _des_histequal.row(des_row_mask[i]).copyTo(des_filter.row(i));
  }
  else
  {
    if (_kps.empty())
      return;
    std::vector<unsigned int> des_row_mask;
    auto n_kps = _kps.size();
    for (int i = 0; i < n_kps; i++)
    {
      bool is_in_BB = false;
      for (auto ROI_features : ROIs_features)
        is_in_BB = is_in_BB || ROI_features.contains(_kps[i].pt);

      if (is_in_BB)
      {
        kps_filter.push_back(_kps[i]);
        des_row_mask.push_back(i);
      }
    }

    des_filter.create(des_row_mask.size(),_des.cols,_des.type());
    auto n_des = des_row_mask.size();
    for (int i = 0; i < n_des; i++)
      _des.row(des_row_mask[i]).copyTo(des_filter.row(i));
  }
}

std::string Image::getID()
{
  return _img_id;
}

bool Image::hasFeatures()
{
  return !_kps.empty();
}




cv::Mat equalizeHist(const cv::Mat &img)
{
  std::vector<cv::Mat> img_channels;
  cv::split(img,img_channels);

  for (auto img_channel : img_channels)
    cv::equalizeHist(img_channel,img_channel);

  cv::Mat img_equal;
  cv::merge(img_channels,img_equal);
  return img_equal;
}
