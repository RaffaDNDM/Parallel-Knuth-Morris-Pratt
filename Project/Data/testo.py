import random
import string

'''
Random string without occorrences
'''
def random_text_no(length=1):
    letters = string.ascii_letters
    return ''.join(random.choice(letters) for i in range(length))

'''
Insertion of occorrences in only strings of cycle 1
'''
def random_text1(length, pattern, cores):
    m = len(pattern)
    size_rank = (length-m+1)//cores
    list_index = [random.randint(i*size_rank,((i+1)*size_rank)-m) for i in range(cores)]
    letters = string.ascii_letters
    print("testo_random1.txt "+str(list_index))

    i=0
    j=0
    final_text = ""

    while(j < cores):
        final_text1 = ""
        final_text2 = ""
        final_text3 = ""

        #print("index1 :"+str(i))
        if(i<list_index[j]):
            final_text1=''.join(random.choice(letters) for k in range(i, list_index[j]))
            i=list_index[j]
            #i=i+list_index[j]
            #print("index2 :"+str(i))

        if(i==list_index[j]):
            final_text2=pattern
            i=i+m
            #print("index3 :"+str(i))

        if(j==(cores-1) and list_index[cores-1]<i and i<length):
            final_text3=''.join(random.choice(letters) for k in range(length-i))
            #print("index4 :"+str(i))

        j=j+1
        final_text=final_text+final_text1+final_text2+final_text3

    return final_text

'''
Insertion of occorrences in only strings of cycle 2
'''
def random_text2(length, pattern, cores):
    m = len(pattern)
    size_rank = (length-m+1)//cores
    list_index = [random.randint(((i+1)*size_rank)-m+1,(((i+1)*size_rank)-1)) for i in range(cores)]
    letters = string.ascii_letters
    print("testo_random2.txt "+str(list_index))

    i=0
    j=0
    final_text = ""

    while(j < cores):
        final_text1 = ""
        final_text2 = ""
        final_text3 = ""

        if(i<list_index[j]):
            final_text1=''.join(random.choice(letters) for k in range(i, list_index[j]))
            i=list_index[j]
            #i=i+list_index[j]

        if(i==list_index[j]):
            final_text2=pattern
            i=i+m

        if(j==(cores-1) and list_index[cores-1]<i and i<length):
            final_text3=''.join(random.choice(letters) for k in range(length-i))

        j=j+1
        final_text=final_text+final_text1+final_text2+final_text3

    return final_text


'''
Insertion of occorrences in either strings of cycle 1 and 2
'''
def random_text12(length, pattern, cores):

    m = len(pattern)
    size_rank = (length-m+1)//cores
    list_index = [random_num(i, m, size_rank) for i in range(2*cores)]
    letters = string.ascii_letters
    print("testo_random12.txt "+str(list_index))

    i=0
    j=0
    final_text = ""

    while(j < 2*cores):
        final_text1 = ""
        final_text2 = ""
        final_text3 = ""

        #print("index1 :"+str(i))
        if(i<list_index[j]):
            final_text1=''.join(random.choice(letters) for k in range(i, list_index[j]))
            #i=i+list_index[j]
            i=list_index[j]
            #print("index2 :"+str(i))

        if(i==list_index[j]):
            final_text2=pattern
            i=i+m
            #print("index3 :"+str(i))

        if(j==2*cores-1 and list_index[2*cores-1]<i and i<length):
            final_text3=''.join(random.choice(letters) for k in range(length-i))
            #print("index4 :"+str(i))

        j=j+1
        final_text=final_text+final_text1+final_text2+final_text3

    return final_text

def random_num(iter, m, size_rank):

    #i=rank processore
    if(iter<2):
        i=0
    else:
        i=iter//2


    if(iter%2==0):
        return random.randint(i*size_rank,((i+1)*size_rank)-(2*m-1))
    else:
        return random.randint(((i+1)*size_rank)-m+1,(((i+1)*size_rank)-1))


#cores=32
cores=4
#length = 1073741824
length = 1024
pattern = "ciao"
#pattern = "AcnjdsDAbABBACDaDCV"

text=random_text_no(length)
print("count "+str(text.count(pattern))+"\n\n")
file = open("./Power_Seven/testo_random.txt", "w")
file.write(text)
file.close()

text=random_text1(length, pattern, cores)
print("count "+str(text.count(pattern))+"\n\n")
file = open("./Power_Seven/testo_random1.txt", "w")
file.write(text)
file.close()

text=random_text2(length, pattern, cores)
print("count "+str(text.count(pattern))+"\n\n")
file = open("./Power_Seven/testo_random2.txt", "w")
file.write(text)
file.close()

text=random_text12(length, pattern, cores)
print("count "+str(text.count(pattern))+"\n\n")
file = open("./Power_Seven/testo_random12.txt", "w")
file.write(text)
file.close()
