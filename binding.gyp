{
    'targets': [
        {
            'target_name': 'unix_stream',
            'sources': [ 'src/unix_stream.cpp' ],
            'include_dirs': [
                '<!(node -e "require(\'nan\')")'
            ]
        }
    ]
}
