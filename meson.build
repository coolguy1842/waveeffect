project('waveeffect', 'cpp')

executable(
    'waveeffect',
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
