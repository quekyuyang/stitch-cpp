#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include "image_struct.hpp"
#include "stitch_compute.hpp"

cv::Mat findStitchParams(std::map<std::string,cv::Mat> &imgs,const int nfeatures,
                         const int max_error_inliers,const int min_n_inliers)
{
  std::vector<Image> images;
  for (const auto &[ID,img] : imgs)
  {
    cv::Mat img_histequal = equalizeHist(img);
    Image image(img_histequal,nfeatures,ID);
    if (!image.hasFeatures())
    {
      std::cout << "No features found" << std::endl;
      return cv::Mat();
    }

    images.push_back(image);
  }
  
  cv::Rect ROI_features1(340,0,300,images[0].getImg().cols-1);
  cv::Rect ROI_features2(1100,0,200,images[0].getImg().cols-1);
  std::vector<cv::Rect> ROIs_features{ROI_features1,ROI_features2};

  std::vector<cv::KeyPoint> kps1,kps2;
  cv::Mat des1,des2;
  images[0].getKpsAndDes(kps1,des1,ROIs_features);
  images[1].getKpsAndDes(kps2,des2,ROIs_features);
  
  std::vector<cv::DMatch> matches;
  getMatches(des1,des2,matches);
  
  std::vector<cv::Point2f> pts1_match,pts2_match; 
  for (const auto &match : matches)
  {
    pts1_match.push_back(kps1[match.queryIdx].pt);
    pts2_match.push_back(kps2[match.trainIdx].pt);
  }
  
  cv::Mat mask;
  auto homo_mat = cv::findHomography(pts2_match,pts1_match,cv::RANSAC,max_error_inliers,mask);
  
  cv::Mat stitch_img;
  cv::Size stitch_size(1844,1500);
  cv::warpPerspective(images[1].getImg(),stitch_img,homo_mat,stitch_size);
  
  cv::Mat ROI = stitch_img.rowRange(0,1032);
  images[0].getImg().copyTo(ROI);
  
  return stitch_img;
}

void getMatches(cv::Mat des1,cv::Mat des2,std::vector<cv::DMatch> &matches)
{
  cv::BFMatcher bf(cv::NORM_HAMMING,true);
  bf.match(des1,des2,matches);
  std::sort(matches.begin(),matches.end());
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
