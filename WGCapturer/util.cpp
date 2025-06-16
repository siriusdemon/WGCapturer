#include <wincodec.h> // WIC


#include <string>
#include <wrl/client.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "windowscodecs.lib")


#include <opencv2/opencv.hpp>

cv::Mat Texture2Mat(ID3D11Device* d3dDevice, ID3D11Texture2D* texture) {
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    d3dDevice->GetImmediateContext(&context);

    D3D11_TEXTURE2D_DESC desc = {};
    texture->GetDesc(&desc);

    D3D11_TEXTURE2D_DESC stagingDesc = desc;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTex;
    HRESULT hr = d3dDevice->CreateTexture2D(&stagingDesc, nullptr, &stagingTex);
    if (FAILED(hr)) return cv::Mat();

    context->CopyResource(stagingTex.Get(), texture);

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = context->Map(stagingTex.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (FAILED(hr)) return cv::Mat();

    int width = desc.Width;
    int height = desc.Height;
    int rowPitch = mapped.RowPitch;

    cv::Mat bgraImage(height, width, CV_8UC4);
    BYTE* srcData = static_cast<BYTE*>(mapped.pData);

    for (int y = 0; y < height; ++y) {
        memcpy(bgraImage.ptr(y), srcData + y * rowPitch, width * 4); 
    }

    context->Unmap(stagingTex.Get(), 0);

    cv::Mat bgrImage;
    cv::cvtColor(bgraImage, bgrImage, cv::COLOR_BGRA2BGR);
    return bgrImage.clone();
}
