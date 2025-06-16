#pragma once
#include "pch.h"
#include <opencv2/opencv.hpp>

cv::Mat Texture2Mat(ID3D11Device* d3dDevice, ID3D11Texture2D* texture);
