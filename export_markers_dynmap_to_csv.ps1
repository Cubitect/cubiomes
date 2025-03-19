cd build
cmake ..
if ($?) {
    cmake --build .
    if ($?) {
        .\export_markers_dynmap_to_csv.exe
    }
}
cd ..