docker build -t raytracing .
docker run --name raytracing-builder raytracing
docker cp raytracing-builder:/app/build ./
docker rm raytracing-builder
cd build
RayTracing.exe
pause