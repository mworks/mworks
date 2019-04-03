class MySillyClass:
    def __str__(self):
        raise RuntimeError('conversion to string failed')

try:
    message(MySillyClass())
finally:
    del MySillyClass
