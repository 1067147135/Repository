def fix(string):
    special_characters = ["\\", "'", "\"", "\0", "\b", "\n", "\r", "\t", "\Z", "\%", "\_"]
    # 需要转义的特殊字符列表

    for character in special_characters:
        if character in string:
            string = string.replace(character, "\\" + character)  # 加上转义符进行替换

    return string