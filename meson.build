project('ai-trap-cpp', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++23'])

cc = meson.get_compiler('cpp')
lib_camera_dep  = cc.find_library('camera', required: true,  dirs: ['/usr/local/lib/aarch64-linux-gnu'])
lib_camera_base_dep  = cc.find_library('camera-base', required: true,  dirs: ['/usr/local/lib/aarch64-linux-gnu'])
lib_pisp_dep  = cc.find_library('pisp', required: true, dirs: ['/usr/local/lib/aarch64-linux-gnu'])
lib_ncnn_dep  = cc.find_library('ncnn', required: true, dirs: ['/usr/local/lib/ncnn'])
openmp_dep = dependency('openmp')
boost_deps = dependency('boost', modules: ['system', 'filesystem'])
opencv_deps = dependency('opencv4', required : true, modules: ['core'])
inc_dirs = include_directories(
    ['include',
     '/usr/local/include/libcamera',
     '/usr/local/include/libpisp'
    ])

exe = executable('ai-trap-cpp',
                 'src/main.cpp',
                 'src/detection_camera.cpp',
                 'src/yolo11_ncnn_model.cpp',
                 'src/image_reference.cpp',
                 install : true,
                 dependencies: [lib_camera_dep, lib_camera_base_dep, lib_pisp_dep, boost_deps, opencv_deps, lib_ncnn_dep, openmp_dep],
                 include_directories : inc_dirs,

)

test('basic', exe)
