#include <vector>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include "stitch_assemble.hpp"

cv::Mat stitchImages(const std::vector<cv::Mat> &imgs,
                  const std::vector<cv::Mat> &homo_mats)
{
  auto cumul_homo_mats = accumulateHomoMats(homo_mats);
  std::vector<cv::Mat> verts_sets;
  for (int i = 0; i < imgs.size(); i++)
  {
    const cv::Size img_size(imgs[i].cols,imgs[i].rows);
    verts_sets.push_back(getTransformedVerts(img_size,cumul_homo_mats[i]));
  }

  auto verts_bounds = getVertsBoundaries(verts_sets);
  int width = verts_bounds.at<int>(2) - verts_bounds.at<int>(0);
  int height = verts_bounds.at<int>(3) - verts_bounds.at<int>(1);
  cv::Size stitch_size(width,height);

  std::vector<std::vector<cv::Point>> verts_sets_points;
  for (const auto &verts : verts_sets)
  {
    auto verts_points = C2_to_points(verts);
    verts_sets_points.push_back(verts_points);
  }

  cv::Mat stitched(stitch_size,CV_8UC3);
  for (int i = 0; i < imgs.size(); i++)
  {
    cv::Mat warped_single(stitch_size,CV_8UC3);
    cv::warpPerspective(imgs[i],warped_single,cumul_homo_mats[i],stitch_size);

    cv::Mat mask(stitch_size,CV_8U,cv::Scalar(0));
    cv::drawContours(mask,verts_sets_points,i,cv::Scalar(255),cv::FILLED);

    warped_single.copyTo(stitched,mask);
  }
  return stitched;
}

cv::Mat getTransformedVerts(const cv::Size &img_size,const cv::Mat &homo_mat)
{
  cv::Mat verts(1,4,CV_32FC2);

  // Four vertices of image with img_size
  cv::Point2f vert1(0,0);
  cv::Point2f vert2(img_size.width,0);
  cv::Point2f vert3(img_size.width,img_size.height);
  cv::Point2f vert4(0,img_size.height);

  std::vector<cv::Point2f> verts_vec{vert1,vert2,vert3,vert4};
  int i = 0;
  for (auto it=verts.begin<cv::Vec2f>(); it !=verts.end<cv::Vec2f>(); ++it)
  {
    (*it)[0] = verts_vec[i].x;
    (*it)[1] = verts_vec[i].y;
    ++i;
  }
  cv::Mat verts_trans(1,4,CV_32FC2);
  cv::perspectiveTransform(verts,verts_trans,homo_mat);
  return verts_trans;
}

cv::Mat getVertsBoundaries(const std::vector<cv::Mat> verts_sets)
{
  // Convert vector of Mat into one Mat to use cv::reduce
  cv::Mat verts_all(verts_sets.size(),4,CV_32FC2);
  for (int i=0; i<verts_sets.size(); i++)
  {
    auto vert_entry = verts_all.row(i);
    verts_sets[i].copyTo(vert_entry);
  }

  cv::Mat low_bounds;
  cv::Mat high_bounds;
  cv::reduce(verts_all,low_bounds,0,cv::REDUCE_MIN);
  cv::reduce(verts_all,high_bounds,0,cv::REDUCE_MAX);
  cv::reduce(low_bounds.reshape(1,4),low_bounds,0,cv::REDUCE_MIN);
  cv::reduce(high_bounds.reshape(1,4),high_bounds,0,cv::REDUCE_MAX);

  cv::Mat verts_bounds(1,4,CV_32F);
  cv::hconcat(low_bounds,high_bounds,verts_bounds);
  verts_bounds.convertTo(verts_bounds,CV_32S);

  return verts_bounds;
}

std::vector<cv::Mat> accumulateHomoMats(const std::vector<cv::Mat> &homo_mats)
{
  auto cumul_homo_mat = cv::Mat::eye(3,3,CV_64F);
  std::vector<cv::Mat> cumul_homo_mats;
  for (const auto &homo_mat : homo_mats)
  {
    cumul_homo_mat = cumul_homo_mat * homo_mat;
    cumul_homo_mats.push_back(cumul_homo_mat);
  }
  return cumul_homo_mats;
}

std::vector<cv::Point> C2_to_points(const cv::Mat C2_mat)
{
  std::vector<cv::Point> points;
  cv::MatConstIterator_<cv::Vec2f> it,end;
  for (it=C2_mat.begin<cv::Vec2f>(), end=C2_mat.end<cv::Vec2f>(); it!=end; ++it)
  {
    int x = std::round((*it)[0]);
    int y = std::round((*it)[1]);
    points.push_back(cv::Point(x,y));
  }
  return points;
}
