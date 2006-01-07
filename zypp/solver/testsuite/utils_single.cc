#include <string.h>
#include <zypp/solver/temporary/utils.h>

using namespace zypp::solver::detail;

int
main (int argc, char *argv[])
{
    if (strstrip(NULL) != NULL)
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }
    if (strstrip("") != NULL)
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strstrip(" ") != NULL)
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strstrip("\t") != NULL)
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strstrip("  ") != NULL)
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strstrip("\t\t") != NULL)
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strstrip(" \t ") != NULL)
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strcmp (strstrip("a"), "a"))
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strcmp (strstrip(" a"), "a"))
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strcmp (strstrip("a "), "a"))
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strcmp (strstrip(" a "), "a"))
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strcmp (strstrip("ab"), "ab"))
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strcmp (strstrip(" ab"), "ab"))
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strcmp (strstrip("ab "), "ab"))
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strcmp (strstrip(" ab "), "ab"))
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strcmp (strstrip("a b"), "a b"))
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strcmp (strstrip(" a b"), "a b"))
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strcmp (strstrip("a b "), "a b"))
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	
    if (strcmp (strstrip(" a b "), "a b"))
    {
	printf ("FAIL\n");
    }
    else
    {
	printf ("PASS\n");	
    }	

    return 0;
}
