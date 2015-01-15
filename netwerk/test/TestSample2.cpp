
#include "TestHarness.h"
#include "nsISample2.h"
#include "prerror.h"

using namespace mozilla;

int
main()
{
  ScopedXPCOM xpcom("Sample2");

  if (xpcom.failed()) {
    fail("Unable to initialize XPCOM");
    return -1;
  }

  nsresult rv;

  nsCOMPtr<nsISample2> sample = do_CreateInstance("@mozilla.org/sample2;1", &rv);
  if (NS_FAILED(rv)) {
    fail("Failed to create sample2.");
	printf("Error code: 0x%X\n", rv);
    return -1;
  }


  rv = sample->WriteValue("Inital print:");
  if (NS_FAILED(rv)) {
    printf("ERROR: Calling nsISample2::WriteValue() [%x]\n",
           static_cast<uint32_t>(rv));
	printf("Error code: 0x%X\n", rv);
    return -3;
  }

  const char* testValue = "XPCOM defies gravity";
  rv = sample->SetValue(testValue);
  if (NS_FAILED(rv)) {
    printf("ERROR: Calling nsISample2::SetValue() [%x]\n",
           static_cast<uint32_t>(rv));
	printf("Error code: 0x%X\n", rv);
    return -3;
  }
  printf("Set value to: %s\n", testValue);
  char* str;
  rv = sample->GetValue(&str);

  if (NS_FAILED(rv)) {
    printf("ERROR: Calling nsISample2::GetValue() [%x]\n",
           static_cast<uint32_t>(rv));
	printf("Error code: 0x%X\n", rv);
    return -3;
  }
  if (strcmp(str, testValue)) {
    printf("Test FAILED.\n");
    return -4;
  }

  NS_Free(str);

  rv = sample->WriteValue("Final print :");
  printf("Test passed.\n");

  passed("Sample2 Works");

  return 0;
}
