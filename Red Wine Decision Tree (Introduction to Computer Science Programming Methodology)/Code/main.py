while True:
    print('Welcome to the main entrance of all the codes of our group!')
    print('If you want to exit, enter "0"')
    print('If you want to view the output of "try_prepruning_ver1.py", enter "1".')
    print('If you want to view the output of "try_prepruning_ver2.py", enter "2".')
    print('If you want to view the output of "try_prepruning_ver3.py", enter "3".')
    print('If you want to view the output of "try_nonpruning_ver1.py", enter "4".')
    print('If you want to view the output of "try_nonpruning_ver2.py", enter "5".')
    print('If you want to view the output of "pruning_ver1.py", enter "6".')
    key = input('Give me your choice:')
    if key == '0':
        break
    elif key == '1':
        print('"try_prepruning_ver1.py" will be executed automatically...')
        import try_prepruning_ver1
    elif key == '2':
        print('"try_prepruning_ver2.py" will be executed automatically...')
        import try_prepruning_ver2
    elif key == '3':
        print('"try_prepruning_ver3.py" will be executed automatically...')
        import try_prepruning_ver3
    elif key == '4':
        print('"try_nonpruning_ver1.py" will be executed automatically...')
        print('This program takes a lot of time to run, please be patient...')
        import try_nonpruning_ver1
    elif key == '5':
        print('"try_nonpruning_ver2.py" will be executed automatically...')
        print('This program takes a lot of time to run, please be patient...')
        import try_nonpruning_ver2
    elif key == '6':
        print('"pruning_ver1.py" will be executed automatically...')
        print('This program takes a lot of time to run, please be patient...')
        import pruning_ver1
    else:
        print('Invalid input! Please re_enter!')
