import wcwidth


def get_console_str_width(string):
    total_width = 0
    for char in string:
        char_width = wcwidth.wcwidth(char)
        if char_width < 0:
            total_width += 1
        else:
            total_width += char_width
    return total_width
