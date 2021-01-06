#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "image_struct.hpp"

Image::Image(const cv::Mat img, int nfeatures, std::string img_id)
            :_img(img), _img_id(img_id)
{
  findFeatures(nfeatures);
}

void Image::findFeatures(int nfeatures)
{
  cv::Ptr<cv::ORB> orb = cv::ORB::create(nfeatures);
  orb->detectAndCompute(_img,cv::noArray(),_kps,_des,false);
}

cv::Mat Image::getImg()
{
  return _img;
}

void Image::getKpsAndDes(std::vector<cv::KeyPoint> &kps,cv::Mat &des,std::vector<cv::Rect> ROIs_features)
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
      kps.push_back(_kps[i]);
      des_row_mask.push_back(i);
    }
  }

  des.create(des_row_mask.size(),_des.cols,_des.type());
  auto n_des = des_row_mask.size();
  for (int i = 0; i < n_des; i++)
    _des.row(des_row_mask[i]).copyTo(des.row(i));
}

std::string Image::getID()
{
  return _img_id;
}

bool Image::hasFeatures()
{
  return !_kps.empty();
}
