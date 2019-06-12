import random
import string

def random_text(length=1):
    letters = string.ascii_letters
    return ''.join(random.choice(letters) for i in range(length))

length = 1024
file1 = open("testo_random1024.txt", "w")
file1.write(random_text(length))
#print((file1.read()).count("AcnjdsDAbABBACDaDCV"))


def random_text(length, pattern, cores, list_index):
    letters = string.ascii_letters

    i=0
    j=0
    final_text = ""

    while(j < cores):
        final_text1 = ""
        final_text2 = ""
        final_text3 = ""

        if(i<list_index[j]):
            final_text1=''.join(random.choice(letters) for k in range(list_index[j]))
            i=i+list_index[j]

        if(i==list_index[j]):
            final_text2=''.join(pattern[k] for k in range(m))
            i=i+m

        if(j==(cores-1) and list_index[cores-1]<i and i<length):
            final_text3=''.join(random.choice(letters) for k in range(length-i))

        j=j+1
        final_text=final_text+final_text1+final_text2+final_text3

    return final_text


#cores=32
cores=4
#length = 1073741824
length = 1024
pattern = "ciao"
#pattern = "AcnjdsDAbABBACDaDCV"
m = len(pattern)
size_rank = (length-m+1)//cores

list_index = [random.randint(i*size_rank,((i+1)*size_rank)-m) for i in range(cores)]
print(list_index)

text=random_text(length, pattern, cores, list_index)
print(text)
print(len(text))
file1 = open("testo_random1.txt", "w")
file1.write()
file1.close()
