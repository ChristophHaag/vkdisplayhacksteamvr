project(
        'vkdisplayinfo',
        ['c'],
        version: '0.1',
        license: 'BSL-1.0',
        meson_version: '>=0.49.0',
        default_options: [
                'warning_level=3',
        ],
)

vulkan_dep = dependency('vulkan', required: true)

executable('vkdisplayinfo',
	'vkdisplayinfo.c',
	dependencies: vulkan_dep
)
