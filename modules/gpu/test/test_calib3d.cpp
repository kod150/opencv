/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                        Intel License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000, Intel Corporation, all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of Intel Corporation may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "test_precomp.hpp"

#ifdef HAVE_CUDA

namespace {

//////////////////////////////////////////////////////////////////////////
// StereoBM

struct StereoBM : testing::TestWithParam<cv::gpu::DeviceInfo>
{
    cv::gpu::DeviceInfo devInfo;

    virtual void SetUp()
    {
        devInfo = GetParam();

        cv::gpu::setDevice(devInfo.deviceID());
    }
};

TEST_P(StereoBM, Regression)
{
    try
    {
        cv::Mat left_image  = readImage("stereobm/aloe-L.png", cv::IMREAD_GRAYSCALE);
        cv::Mat right_image = readImage("stereobm/aloe-R.png", cv::IMREAD_GRAYSCALE);
        cv::Mat disp_gold   = readImage("stereobm/aloe-disp.png", cv::IMREAD_GRAYSCALE);

        ASSERT_FALSE(left_image.empty());
        ASSERT_FALSE(right_image.empty());
        ASSERT_FALSE(disp_gold.empty());

        cv::gpu::StereoBM_GPU bm(0, 128, 19);
        cv::gpu::GpuMat disp;

        bm(loadMat(left_image), loadMat(right_image), disp);

        EXPECT_MAT_NEAR(disp_gold, disp, 0.0);
    }
    catch (...)
    {
        cv::gpu::resetDevice();
        throw;
    }
}

INSTANTIATE_TEST_CASE_P(GPU_Calib3D, StereoBM, ALL_DEVICES);

//////////////////////////////////////////////////////////////////////////
// StereoBeliefPropagation

struct StereoBeliefPropagation : testing::TestWithParam<cv::gpu::DeviceInfo>
{
    cv::gpu::DeviceInfo devInfo;

    virtual void SetUp()
    {
        devInfo = GetParam();

        cv::gpu::setDevice(devInfo.deviceID());
    }
};

TEST_P(StereoBeliefPropagation, Regression)
{
    try
    {
        cv::Mat left_image  = readImage("stereobp/aloe-L.png");
        cv::Mat right_image = readImage("stereobp/aloe-R.png");
        cv::Mat disp_gold   = readImage("stereobp/aloe-disp.png", cv::IMREAD_GRAYSCALE);

        ASSERT_FALSE(left_image.empty());
        ASSERT_FALSE(right_image.empty());
        ASSERT_FALSE(disp_gold.empty());

        cv::gpu::StereoBeliefPropagation bp(64, 8, 2, 25, 0.1f, 15, 1, CV_16S);
        cv::gpu::GpuMat disp;

        bp(loadMat(left_image), loadMat(right_image), disp);

        cv::Mat h_disp(disp);
        h_disp.convertTo(h_disp, disp_gold.depth());

        EXPECT_MAT_NEAR(disp_gold, h_disp, 0.0);
    }
    catch (...)
    {
        cv::gpu::resetDevice();
        throw;
    }
}

INSTANTIATE_TEST_CASE_P(GPU_Calib3D, StereoBeliefPropagation, ALL_DEVICES);

//////////////////////////////////////////////////////////////////////////
// StereoConstantSpaceBP

struct StereoConstantSpaceBP : testing::TestWithParam<cv::gpu::DeviceInfo>
{
    cv::gpu::DeviceInfo devInfo;

    virtual void SetUp()
    {
        devInfo = GetParam();

        cv::gpu::setDevice(devInfo.deviceID());
    }
};

TEST_P(StereoConstantSpaceBP, Regression)
{
    try
    {
        cv::Mat left_image  = readImage("csstereobp/aloe-L.png");
        cv::Mat right_image = readImage("csstereobp/aloe-R.png");

        cv::Mat disp_gold;

        if (supportFeature(devInfo, cv::gpu::FEATURE_SET_COMPUTE_20))
            disp_gold = readImage("csstereobp/aloe-disp.png", cv::IMREAD_GRAYSCALE);
        else
            disp_gold = readImage("csstereobp/aloe-disp_CC1X.png", cv::IMREAD_GRAYSCALE);

        ASSERT_FALSE(left_image.empty());
        ASSERT_FALSE(right_image.empty());
        ASSERT_FALSE(disp_gold.empty());

        cv::gpu::StereoConstantSpaceBP csbp(128, 16, 4, 4);
        cv::gpu::GpuMat disp;

        csbp(loadMat(left_image), loadMat(right_image), disp);

        cv::Mat h_disp(disp);
        h_disp.convertTo(h_disp, disp_gold.depth());

        EXPECT_MAT_NEAR(disp_gold, h_disp, 1.0);
    }
    catch (...)
    {
        cv::gpu::resetDevice();
        throw;
    }
}

INSTANTIATE_TEST_CASE_P(GPU_Calib3D, StereoConstantSpaceBP, ALL_DEVICES);

///////////////////////////////////////////////////////////////////////////////////////////////////////
// transformPoints

struct TransformPoints : testing::TestWithParam<cv::gpu::DeviceInfo>
{
    cv::gpu::DeviceInfo devInfo;

    virtual void SetUp()
    {
        devInfo = GetParam();

        cv::gpu::setDevice(devInfo.deviceID());
    }
};

TEST_P(TransformPoints, Accuracy)
{
    try
    {
        cv::Mat src = randomMat(cv::Size(1000, 1), CV_32FC3, 0, 10);
        cv::Mat rvec = randomMat(cv::Size(3, 1), CV_32F, 0, 1);
        cv::Mat tvec = randomMat(cv::Size(3, 1), CV_32F, 0, 1);

        cv::gpu::GpuMat dst;
        cv::gpu::transformPoints(loadMat(src), rvec, tvec, dst);

        ASSERT_EQ(src.size(), dst.size());
        ASSERT_EQ(src.type(), dst.type());

        cv::Mat h_dst(dst);

        cv::Mat rot;
        cv::Rodrigues(rvec, rot);

        for (int i = 0; i < h_dst.cols; ++i)
        {
            cv::Point3f res = h_dst.at<cv::Point3f>(0, i);

            cv::Point3f p = src.at<cv::Point3f>(0, i);
            cv::Point3f res_gold(
                    rot.at<float>(0, 0) * p.x + rot.at<float>(0, 1) * p.y + rot.at<float>(0, 2) * p.z + tvec.at<float>(0, 0),
                    rot.at<float>(1, 0) * p.x + rot.at<float>(1, 1) * p.y + rot.at<float>(1, 2) * p.z + tvec.at<float>(0, 1),
                    rot.at<float>(2, 0) * p.x + rot.at<float>(2, 1) * p.y + rot.at<float>(2, 2) * p.z + tvec.at<float>(0, 2));

            ASSERT_POINT3_NEAR(res_gold, res, 1e-5);
        }
    }
    catch (...)
    {
        cv::gpu::resetDevice();
        throw;
    }
}

INSTANTIATE_TEST_CASE_P(GPU_Calib3D, TransformPoints, ALL_DEVICES);

///////////////////////////////////////////////////////////////////////////////////////////////////////
// ProjectPoints

struct ProjectPoints : testing::TestWithParam<cv::gpu::DeviceInfo>
{
    cv::gpu::DeviceInfo devInfo;

    virtual void SetUp()
    {
        devInfo = GetParam();

        cv::gpu::setDevice(devInfo.deviceID());
    }
};

TEST_P(ProjectPoints, Accuracy)
{
    try
    {
        cv::Mat src = randomMat(cv::Size(1000, 1), CV_32FC3, 0, 10);
        cv::Mat rvec = randomMat(cv::Size(3, 1), CV_32F, 0, 1);
        cv::Mat tvec = randomMat(cv::Size(3, 1), CV_32F, 0, 1);
        cv::Mat camera_mat = randomMat(cv::Size(3, 3), CV_32F, 0.5, 1);
        camera_mat.at<float>(0, 1) = 0.f;
        camera_mat.at<float>(1, 0) = 0.f;
        camera_mat.at<float>(2, 0) = 0.f;
        camera_mat.at<float>(2, 1) = 0.f;

        cv::gpu::GpuMat dst;
        cv::gpu::projectPoints(loadMat(src), rvec, tvec, camera_mat, cv::Mat(), dst);

        ASSERT_EQ(1, dst.rows);
        ASSERT_EQ(MatType(CV_32FC2), MatType(dst.type()));

        std::vector<cv::Point2f> dst_gold;
        cv::projectPoints(src, rvec, tvec, camera_mat, cv::Mat(1, 8, CV_32F, cv::Scalar::all(0)), dst_gold);

        ASSERT_EQ(dst_gold.size(), static_cast<size_t>(dst.cols));

        cv::Mat h_dst(dst);

        for (size_t i = 0; i < dst_gold.size(); ++i)
        {
            cv::Point2f res = h_dst.at<cv::Point2f>(0, (int)i);
            cv::Point2f res_gold = dst_gold[i];

            ASSERT_LE(cv::norm(res_gold - res) / cv::norm(res_gold), 1e-3f);
        }
    }
    catch (...)
    {
        cv::gpu::resetDevice();
        throw;
    }
}

INSTANTIATE_TEST_CASE_P(GPU_Calib3D, ProjectPoints, ALL_DEVICES);

///////////////////////////////////////////////////////////////////////////////////////////////////////
// SolvePnPRansac

struct SolvePnPRansac : testing::TestWithParam<cv::gpu::DeviceInfo>
{
    cv::gpu::DeviceInfo devInfo;

    virtual void SetUp()
    {
        devInfo = GetParam();

        cv::gpu::setDevice(devInfo.deviceID());
    }
};

TEST_P(SolvePnPRansac, Accuracy)
{
    try
    {
        cv::Mat object = randomMat(cv::Size(5000, 1), CV_32FC3, 0, 100);
        cv::Mat camera_mat = randomMat(cv::Size(3, 3), CV_32F, 0.5, 1);
        camera_mat.at<float>(0, 1) = 0.f;
        camera_mat.at<float>(1, 0) = 0.f;
        camera_mat.at<float>(2, 0) = 0.f;
        camera_mat.at<float>(2, 1) = 0.f;

        std::vector<cv::Point2f> image_vec;
        cv::Mat rvec_gold;
        cv::Mat tvec_gold;
        rvec_gold = randomMat(cv::Size(3, 1), CV_32F, 0, 1);
        tvec_gold = randomMat(cv::Size(3, 1), CV_32F, 0, 1);
        cv::projectPoints(object, rvec_gold, tvec_gold, camera_mat, cv::Mat(1, 8, CV_32F, cv::Scalar::all(0)), image_vec);

        cv::Mat rvec, tvec;
        std::vector<int> inliers;
        cv::gpu::solvePnPRansac(object, cv::Mat(1, (int)image_vec.size(), CV_32FC2, &image_vec[0]),
                                camera_mat, cv::Mat(1, 8, CV_32F, cv::Scalar::all(0)),
                                rvec, tvec, false, 200, 2.f, 100, &inliers);

        ASSERT_LE(cv::norm(rvec - rvec_gold), 1e-3);
        ASSERT_LE(cv::norm(tvec - tvec_gold), 1e-3);
    }
    catch (...)
    {
        cv::gpu::resetDevice();
        throw;
    }
}

INSTANTIATE_TEST_CASE_P(GPU_Calib3D, SolvePnPRansac, ALL_DEVICES);

////////////////////////////////////////////////////////////////////////////////
// reprojectImageTo3D

PARAM_TEST_CASE(ReprojectImageTo3D, cv::gpu::DeviceInfo, cv::Size, MatDepth, UseRoi)
{
    cv::gpu::DeviceInfo devInfo;
    cv::Size size;
    int depth;
    bool useRoi;

    virtual void SetUp()
    {
        devInfo = GET_PARAM(0);
        size = GET_PARAM(1);
        depth = GET_PARAM(2);
        useRoi = GET_PARAM(3);

        cv::gpu::setDevice(devInfo.deviceID());
    }
};

TEST_P(ReprojectImageTo3D, Accuracy)
{
    try
    {
        cv::Mat disp = randomMat(size, depth, 5.0, 30.0);
        cv::Mat Q = randomMat(cv::Size(4, 4), CV_32FC1, 0.1, 1.0);

        cv::gpu::GpuMat dst;
        cv::gpu::reprojectImageTo3D(loadMat(disp, useRoi), dst, Q, 3);

        cv::Mat dst_gold;
        cv::reprojectImageTo3D(disp, dst_gold, Q, false);

        EXPECT_MAT_NEAR(dst_gold, dst, 1e-5);
    }
    catch (...)
    {
        cv::gpu::resetDevice();
        throw;
    }
}

INSTANTIATE_TEST_CASE_P(GPU_Calib3D, ReprojectImageTo3D, testing::Combine(
    ALL_DEVICES,
    DIFFERENT_SIZES,
    testing::Values(MatDepth(CV_8U), MatDepth(CV_16S)),
    WHOLE_SUBMAT));

} // namespace

#endif // HAVE_CUDA
