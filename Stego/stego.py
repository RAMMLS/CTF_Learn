from stegano import lsb

# для начала мы прячем наше послание. Передаем параметры:
# 1) исходная картинка, куда будем прятать
# 2) Наше секретное сообщение

secret = lsb.hide("~/Downloads/GigaChad.png", "My secret is...")

# сохраняем наше зашифрованное изображение уже с другим названием

secret.save("img/final.png")

# А здесь мы раскрываем наше секретное сообщение. Здесь в выводе будет "My secret is..."

print(lsb.reveal("img/final.png"))
