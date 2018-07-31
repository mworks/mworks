from urllib.request import urlopen


with urlopen('https://duckduckgo.com') as resp:
    assert resp.getcode() == 200
