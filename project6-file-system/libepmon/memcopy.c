void *memcopy(void *s1, const void *s2, int n) 
{
        const char *f = s2;
        char *t = s1;         

        if (f < t) {          
                f += n;
                t += n;
                while (n-- > 0)                
                        *--t = *--f;                   
        } else
                while (n-- > 0)                
                        *t++ = *f++;                   
        return s1;
}



#if 0
void memcopy(unsigned char *dest, unsigned char *src, int len)
{
        for (; len != 0; len--)        
        {
                *dest++ = *src++;              
        }
}

#endif
