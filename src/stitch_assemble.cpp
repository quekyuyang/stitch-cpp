#include <vector>
#include <opencv2/opencv.hpp>
#include <iostream>
#include "stitch_assemble.hpp"

void stitchImages(const std::vector<cv::Mat> &imgs,
                  const std::vector<cv::Mat> &homo_mats)
{
  //auto cumul_homo_mats = accumulateHomoMats(homo_mats);
  for (int i = 0; i < imgs.size()-1; i++)
  {
    const auto &img_top = imgs[i];
    const auto &img_bot = imgs[i+1];

    const auto &homo_mat_bot = homo_mats[i+1];
    const auto homo_mat_top = cv::Mat::eye(3,3,homo_mat_bot.type());

    const cv::Size size_top(img_top.cols,img_top.rows);
    const cv::Size size_bot(img_bot.cols,img_bot.rows);

    auto verts_top = getTransformedVerts(size_top,homo_mat_top);
    auto verts_bot = getTransformedVerts(size_bot,homo_mat_bot);
    auto verts_bounds = getVertsBoundaries(
      std::vector<cv::Mat>{verts_top,verts_bot});

    int width = verts_bounds.at<int>(2) - verts_bounds.at<int>(0);
    int height = verts_bounds.at<int>(3) - verts_bounds.at<int>(1);
    cv::Size stitch_size(width,height);

    cv::Mat stitch;
    cv::warpPerspective(img_bot,stitch,homo_mat_bot,stitch_size);
    cv::imwrite("/home/yquek/Downloads/"+std::to_string(i)+"a.jpg",stitch);
    cv::Mat ROI = stitch.rowRange(0,img_top.rows).colRange(0,img_top.cols);
    img_top.copyTo(ROI);
    cv::imwrite("/home/yquek/Downloads/"+std::to_string(i)+"b.jpg",stitch);
  }
}

cv::Mat getTransformedVerts(const cv::Size &img_size,const cv::Mat &homo_mat)
{
  cv::Mat verts(1,4,CV_32FC2);

  // Four vertices of image with img_size
  cv::Point2f vert1(0,0);
  cv::Point2f vert2(img_size.width,0);
  cv::Point2f vert3(0,img_size.height);
  cv::Point2f vert4(img_size.width,img_size.height);
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
  auto cumul_homo_mat = cv::Mat::eye(3,3,CV_32F);
  std::vector<cv::Mat> cumul_homo_mats;
  for (const auto &homo_mat : homo_mats)
  {
    cumul_homo_mat *= homo_mat;
    cumul_homo_mats.push_back(cumul_homo_mat);
  }
  return cumul_homo_mats;
}
