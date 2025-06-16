#pragma once
#include "pch.h"
#include <string>
#include <opencv2/opencv.hpp>

bool SaveTextureToPNG(ID3D11Device* d3dDevice, ID3D11Texture2D* texture, const std::string& filename);

cv::Mat Texture2Mat(ID3D11Device* d3dDevice, ID3D11Texture2D* texture);
