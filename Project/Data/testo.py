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

    final_text=''

    i=0
    j=0
    while(j < cores):
        k=0

        while(i<list_index[j]):
            final_text.join(random.choice(letters))
            i=i+1

        while(i<list_index[j]+m):
            final_text.join(pattern[k])
            i=i+1
            k=k+1

        while(j==(cores-1) and list_index[cores-1]<i and i<length):
            final_text.join(random.choice(letters))
            i=i+1

        j=j+1

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
file1 = open("testo_random1.txt", "w")
#file1.write()
file1.close()
