#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <phone-utils.h>

int
main(int argc, char *argv[])
{
	char *normalized, *number;
	if (argc != 2) {
		fprintf(stderr, "Wrong syntax! Correct syntax is: phoneutils_test <phone_number>\n");
		return 1;
	}

	phone_utils_init();
	
	number = strdup(argv[1]);
	if (!number) {
		perror("strdup number");
		exit(2);
	}
	
	normalized = phone_utils_normalize_number(number);

	if (normalized) {
		printf("Normalized number: %s\n", normalized);
		free(normalized);
	}

	phone_utils_remove_filler_chars(number);
	printf("Filtered number: %s\n", number);

	free(number);
	number = argv[1];

	if (phone_utils_call_is_valid_number(number)) {
		printf("Number is valid for calls.\n");
	}
	else {
		printf("Number is not valid for calls.\n");
	}

	if (phone_utils_sms_is_valid_number(number)) {
		printf("Number is valid for SMS.\n");
	}
	else {
		printf("Number is not valid for SMS.\n");
	}
	
	phone_utils_deinit();
	return 0;
}
