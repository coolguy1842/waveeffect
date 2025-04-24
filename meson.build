project('waveeffect', 'cpp')

executable(
    'wave',
    'src/main.cpp',
    include_directories: include_directories([
        'include',
        'src/include'
    ]),
    dependencies: [
        dependency('hidapi'),
        dependency('libvirt'),
        dependency('libevdev')
    ],
    install: true 
)
