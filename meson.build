project(
        'vkdisplayhacksteamvr',
        ['c'],
        version: '0.1',
        license: 'BSL-1.0',
        meson_version: '>=0.49.0',
        default_options: [
                'warning_level=3',
        ],
)

vulkan_dep = dependency('vulkan', required: true)
xcb_dep = dependency('x11-xcb', required: true)
xcb_randr_dep = dependency('xcb-randr', required: true)

executable('vkdisplayhacksteamvr',
	'vkdisplayinfo.c',
	dependencies: [vulkan_dep, xcb_dep, xcb_randr_dep]
)
