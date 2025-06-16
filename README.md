# Window Graphic Capture Library


Base on [Win32CaptureSample](https://github.com/robmikh/Win32CaptureSample).


### usage
```c++
int main() {
    HWND hwnd = FindWindowA(nullptr, "MapleStory");
    if (!hwnd) {
        printf("MapleStory not found\n");
        return -1;
    }
    WGCapturer grab(hwnd);
    grab.setTargetRegion(1, 31, 1366, 768);
    auto mat = grab.capture({});
    cv::imwrite("my.png", mat[0]);
    return 0;
}
```


