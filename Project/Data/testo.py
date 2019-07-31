import random
import string

'''
Random string without occorrences
def random_no(length=1):
    letters = string.ascii_letters
    return ''.join(random.choice(letters) for i in range(length))
'''

'''
Insertion of occurrences in only strings of cycle 1
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
def random_text2(length, pattern, cores, file):
    m = len(pattern)
    size_rank = (length-m+1)//cores
    list_index = [random.randint(((i+1)*size_rank)-m+1,(((i+1)*size_rank)-1)) for i in range(cores)]
    letters = string.ascii_letters
    print(file+" "+str(list_index))

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

'''
text=random_text_no(length)
print("count "+str(text.count(pattern))+"\n\n")
file = open("testo_random.txt", "w")
file.write(text)
file.close()

text=random_text1(length, pattern, cores)
print("count "+str(text.count(pattern))+"\n\n")
file = open("testo_random1.txt", "w")
file.write(text)
file.close()
list_dim=[1024*1024]

for i in list_dim:
    file_string="testo_random2_"+str(i)+".txt"
    text=random_text2(i, pattern, cores, file_string)
    print("count "+str(text.count(pattern))+"\n\n")
    file = open(file_string, "w")
    file.write(text)
    file.close()

text=random_text12(length, pattern, cores)
print("count "+str(text.count(pattern))+"\n\n")
file = open("testo_random12.txt", "w")
file.write(text)
file.close()
'''

'''
Insertion of one occurrences in the last part of the text for cycle 2
'''
def random_last(length, pattern, cores, block_size, file_string, path):
    file = open(path + "\\" +"Indices.txt", "a+")

    m = len(pattern)
    last_size = length % block_size;
    offset = (length // block_size) * block_size;
    index = offset + random.randint(last_size-(2*m-1), last_size-m)
    letters = string.ascii_letters
    print()
    print

    file.write("---------------------------------------------------------------------------\n")
    file.write("--------------------------"+file_string+"-------------------------\n")
    file.write("---------------------------------------------------------------------------\n")
    file.write("index: "+str(index)+"\n")

    final_text1 = ""
    final_text2 = ""
    final_text3 = ""

    final_text1=''.join(random.choice(letters) for k in range(index))
    final_text2=pattern
    final_text3=''.join(random.choice(letters) for k in range(length-index-m))

    text=final_text1+final_text2+final_text3

    file.write("count "+str(text.count(pattern))+"\n\n")
    file.write("---------------------------------------------------------------------------\n\n")
    file.close()

    file = open(path + "\\" +file_string, "w")
    file.write(text)
    file.close()


def random_no(length,file_string, path):
    print("---------------------------------------------------------------------------\n")
    print("--------------------------"+file_string+"-------------------------\n")
    print("---------------------------------------------------------------------------\n")

    text = ""
    letters = string.ascii_letters
    text=''.join(random.choice(letters) for i in range(length))

    print("count "+str(text.count(pattern))+"\n\n")
    file = open(path + "\\" +file_string, "w")
    file.write(text)
    file.close()


cores=32
#cores=4
block_size=50*1024*1024
#length = 1024
file = open("pattern.txt", "r")
pattern = file.read()
pattern = pattern[:-1] #tolgo l'a capo a fine riga
print(str(pattern))
file.close()
#pattern = "AcnjdsDAbABBACDaDCV"
#list_dim={'1KB' : 1024, '1MB' : 1024*1024, '0.5GB' : 512*1024*1024, '1GB' : 1024*1024*1024, '1.5MB' : 1536*1024*1024}
list_dim={'0.5GB' : 512*1024*1024, '1GB' : 1024*1024*1024, '1.5MB' : 1536*1024*1024}
#list_dim={'1KB' : 1024, '1MB' : 1024*1024}

for key in list_dim:
    file_string_last = "testo_last_"+key+".txt"
    file_string_no = "testo_no_"+key+".txt"
    path_last = ".\\Data\\Last"
    path_no = ".\\Data\\No"
    random_last(list_dim[key], pattern, cores, block_size, file_string_last, path_last)
    random_no(list_dim[key], file_string_no, path_no)
