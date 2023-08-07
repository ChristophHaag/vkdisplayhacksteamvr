project(
        'vkdisplayhacksteamvr',
        ['c', 'cpp'],
        version: '0.1',
        license: 'BSL-1.0',
        meson_version: '>=0.49.0',
        default_options: [
                'warning_level=3',
                'c_std=c11',
                'cpp_std=c++17',
        ],
)

vulkan_dep = dependency('vulkan', required: true)
xcb_dep = dependency('x11-xcb', required: true)
xcb_randr_dep = dependency('xcb-randr', required: true)

executable('vkdisplayhacksteamvr',
	'vkdisplayhacksteamvr.c',
	dependencies: [vulkan_dep, xcb_dep, xcb_randr_dep]
)


layer_sources = files('vkdisplayhacksteamvr_apilayer.cpp', 'vkdisplayhacksteamvr.c')

library('vkdisplayhacksteamvr_apilayer',
        layer_sources,
        dependencies: [vulkan_dep, xcb_dep, xcb_randr_dep]
)

name = 'vkdisplayhacksteamvr_apilayer.json'
custom_target('copy file',
  input : name,
  output :  name,
  command : ['cp', '@INPUT@', '@OUTPUT@'],
  install : false,
  build_by_default : true)
