#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <alloca.h>
#include <assert.h>

struct byte
{
    unsigned char a : 1;
    unsigned char b : 1;
    unsigned char c : 1;
    unsigned char d : 1;
    unsigned char e : 1;
    unsigned char f : 1;
    unsigned char g : 1;
    unsigned char h : 1;
};

union bitfield
{
    struct byte bit;
    unsigned char byte;
};

struct Byte_array
{
    size_t size;
    union bitfield *arr;
};

void BA_init(struct Byte_array *obj,size_t size)
{
    obj->size = size;
    obj->arr = calloc(sizeof(union bitfield),size);
}

void BA_destroy(struct Byte_array *obj)
{
    obj->size = 0;
    free(obj->arr);
}

void BA_bitset(union bitfield *byte, unsigned char value, size_t n)
{
    switch(n)
    {
        case 0: byte->bit.a = value; break;
        case 1: byte->bit.b = value; break;
        case 2: byte->bit.c = value; break;  
        case 3: byte->bit.d = value; break;   
        case 4: byte->bit.e = value; break;
        case 5: byte->bit.f = value; break;
        case 6: byte->bit.g = value; break;
        case 7: byte->bit.h = value; break;
        default: assert(0);
    }
}

unsigned char BA_bitcheck( union bitfield byte, size_t n)
{
    switch(n)
    {
        case 0: return byte.bit.a; break;
        case 1: return byte.bit.b; break;
        case 2: return byte.bit.c; break;  
        case 3: return byte.bit.d; break;   
        case 4: return byte.bit.e; break;
        case 5: return byte.bit.f; break;
        case 6: return byte.bit.g; break;
        case 7: return byte.bit.h; break;
        default: assert(0);
    }
}

void BA_shiftleft(struct Byte_array *obj,size_t size)
{
    assert( size > 0 && size <= 8 );
    obj->arr[0].byte <<= size;
    for( int i=0; i < obj->size-1 ; ++i )
    {
        int k = 0;
        for(int j = 8-size;j<8; ++j)
        {
            BA_bitset( &(obj->arr[i]) , BA_bitcheck(obj->arr[i+1] , j), k++ );
        }
        obj->arr[i+1].byte <<= size;
    }
}

int BA_shift2one( struct Byte_array *obj )
{
    int k = 0;
    while( BA_bitcheck(obj->arr[0],7) == 0 && k != 8*obj->size)
    {
        BA_shiftleft(obj,1);
        ++k;
    }
    return k;
}

void print_bit(struct byte mybit)
{
    printf("Bits: %i %i %i %i %i %i %i %i\n",mybit.h,mybit.g,mybit.f,mybit.e,mybit.d,mybit.c,mybit.b,mybit.a);
}

void BA_print(struct Byte_array obj)
{
    printf("Printing %zu bytes:\n",obj.size);
    for(int i = 0; i<obj.size;++i)
    {
        print_bit(obj.arr[i].bit);
    }
}

void BA_crc(struct Byte_array* obj,struct Byte_array* crc,unsigned char* output)
{
    int len = 8 * obj->size;
    printf("Message:\n");
    BA_print(*obj);
    while(len > 0)
    {
        obj->arr[0].bit.h ^= crc->arr[0].bit.h;
        obj->arr[0].bit.g ^= crc->arr[0].bit.g;
        obj->arr[0].bit.f ^= crc->arr[0].bit.f;
        obj->arr[0].bit.e ^= crc->arr[0].bit.e;
        //printf("Len: %i\n",len);
        len -= BA_shift2one(obj);
        //BA_print(*obj);
        //fflush(stdin);
    }
    *output = obj->arr[0].bit.h*128 + obj->arr[0].bit.g*64 + obj->arr[0].bit.f*32 + obj->arr[0].bit.e*16;
}

int main()
{
    struct Byte_array mybits;
    struct Byte_array divisor;
    
    BA_init(&mybits,3);
    BA_init(&divisor,1);
 
    mybits.arr[0].byte = 201;
    mybits.arr[1].byte = 136;
    mybits.arr[2].byte = 221;
    
    divisor.arr[0].bit.h = 1;
    divisor.arr[0].bit.g = 0;
    divisor.arr[0].bit.f = 1;
    divisor.arr[0].bit.e = 1;
   
    unsigned char output;
    BA_crc(&mybits,&divisor,&output);
    output>>=4;
    printf("CRC: \n");
    print_bit(*((struct byte*)&output));
    BA_destroy(&divisor);
    BA_destroy(&mybits);
    return 0;
}
