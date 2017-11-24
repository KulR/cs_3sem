Задание 1. Запуск процессов по расписанию.
Требуется написать программу mycron, которая будет исполнять программы в заданное
время. Информация о времени и запущенных программах хранится в файле mycrontab.
Формат файла следующий:
10:30:30 cat /usr/include/stdio.h
*:00:00 echo beep
*:*:0 ls -l /
В каждой строке присутствует строка времени (разделённая двоеточиями на часы, ми-
нуты и секунды). В первом и втором поле времени могут быть звёздочки, означающие, что
часы (звёздочка в первой позиции ) и минуты (звёздочка во второй позиции) будут любыми,
например,
*:*:0 ls -l /
должно приводить к запуску ls -l / ровно в 0 секунд каждой минуты.
Файл mycrontab может изменяться при работе программы (например, вы его можете отре-
дактировать для того, чтобы запустить другую программу). Если добавились новые задачи
или удалены старые — требуется завершить все исполняющиеся процессы и запустить всё
по-новому.
Требуется самостоятельно разобрать строки во входном файле и правильно и вовремя
запускать нужные программы. Нельзя использовать функции system, popen и аналогичные,
а так же вызовы оболочки с передачей ей строки выполнения. Все аргументы должны быть
разобраны самостоятельно.
Количество строк во входном файле может быть очень большим, длина любой из строк
может быть очень большой.
Последний срок сдачи задачи №1 — 4 ноября.