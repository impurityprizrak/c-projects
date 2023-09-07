#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

typedef struct Customer
{
    char *id;
    char *name;
    char *email;
    char *phone;
} customer;

typedef struct CustomerData {
    int idx;
    customer *data;
} customerData;

struct CustomerFilter
{
    char *key;
    char *value;
    enum CP{EQUAL, NOT_EQUAL} comp;
    enum CT{OR, AND} closureType;
    int closureLen;
    int *closures;
};

struct UpdateStatement
{
    int id;
    int name;
    int email;
    int phone;
};

char *idGenerator(char *id, int length)
{
    const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    id = malloc((length+1) *sizeof(char));

    srand(time(0));

    for (int i = 0; i < length; i++) {
        int index = rand()%strlen(base);
        int ch = base[index];
        id[i] = ch;
    }

    id[length] = '\0';
    return id;
}

int all(int array[], int length)
{
    int def = 1;

    for (int i = 0; i < length; i++) {
        int value = array[i];

        if (!value) {
            def = 0;
        }
    }

    return def;
}

int any(int array[], int length)
{
    int def = 0;

    for (int i = 0; i < length; i++) {
        int value = array[i];

        if (value) {
            def = 1;
        }
    }

    return def;
}

int filterCustomer(struct Customer item, struct CustomerFilter *filters, int filterSize)
{
    int filterResults[filterSize];
    int tempResults[filterSize];

    for (int i = 0; i < filterSize; i++) {
        struct CustomerFilter filter = filters[i];
        
        if (strcmp(filter.key, "name") == 0) {
            if (filter.comp == EQUAL) {
                filterResults[i] = (strcmp(item.name, filter.value) == 0) ? 1 : 0;
            } else if (filter.comp == NOT_EQUAL) {
                filterResults[i] = (strcmp(item.name, filter.value) != 0) ? 1 : 0;
            }
        } else if (strcmp(filter.key, "email") == 0) {
            if (filter.comp == EQUAL) {
                filterResults[i] = (strcmp(item.email, filter.value) == 0) ? 1 : 0;
            } else if (filter.comp == NOT_EQUAL) {
                filterResults[i] = (strcmp(item.email, filter.value) != 0) ? 1 : 0;
            }
        } else if (strcmp(filter.key, "phone") == 0) {
            if (filter.comp == EQUAL) {
                filterResults[i] = (strcmp(item.phone, filter.value) == 0) ? 1 : 0;
            } else if (filter.comp == NOT_EQUAL) {
                filterResults[i] = (strcmp(item.phone, filter.value) != 0) ? 1 : 0;
            }
        }
    }

    memcpy(tempResults, filterResults, sizeof(filterResults));

    for (int f = 0; f < filterSize; f++) {
        struct CustomerFilter filter = filters[f];
        int closuresResults[filter.closureLen+1];
        closuresResults[0] = tempResults[f];

        int closuresCount = 0;

        for (int c = 0; c < filter.closureLen; c++) {
            closuresResults[c+1] = tempResults[filter.closures[c]];
        }

        if (filter.closureType == AND) {
            tempResults[f] = all(closuresResults, filter.closureLen+1);
        } else {
            tempResults[f] = any(closuresResults, filter.closureLen+1);
        }
    }

    return all(tempResults, filterSize);
}

void updateCustomer(customerData *data, customer fields, struct UpdateStatement statement, struct CustomerFilter *filters, int filterSize)
{
    customer *temp_data = malloc(data->idx * sizeof(customer));
    memcpy(temp_data, data->data, data->idx * sizeof(customer));

    for (int i = 0; i < data->idx; i++) {
        int filtered = filterCustomer(temp_data[i], filters, filterSize);

        if (filtered) {
            if (statement.id == 1) {
                char *new_id = (char *)realloc(temp_data[i].id, strlen(fields.id)+1);
                temp_data[i].id = new_id;
                strcpy(temp_data[i].id, fields.id);
            } 
            if (statement.name == 1) {
                char *new_name = (char *)realloc(temp_data[i].name, strlen(fields.name)+1);
                temp_data[i].name = new_name;
                strcpy(temp_data[i].name, fields.name);
            } 
            if (statement.email == 1) {
                char *new_email = (char *)realloc(temp_data[i].email, strlen(fields.email)+1);
                temp_data[i].email = new_email;
                strcpy(temp_data[i].email, fields.email);
            } 
            if (statement.phone == 1) {
                char *new_phone = (char *)realloc(temp_data[i].phone, strlen(fields.phone)+1);
                temp_data[i].phone = new_phone;
                strcpy(temp_data[i].phone, fields.phone);
            }
        }
    }

    free(data->data);
    data->data = temp_data;
}

void deleteCustomer(customerData *data, struct CustomerFilter *filters, int filterSize)
{
    customer *temp_data = malloc(data->idx * sizeof(customer));
    memcpy(temp_data, data->data, data->idx * sizeof(customer));

    int delete = 0;

    for (int i = 0, c = 0; i < data->idx; i++) {
        int filtered = filterCustomer(temp_data[i], filters, filterSize);

        if (filtered) {
            delete++;
            continue;
        }
        
        temp_data[c] = temp_data[i];
        c+=1;
    }

    data->idx -= delete;
    data->data = realloc(data->data, data->idx * sizeof(customer));
    memcpy(data->data, temp_data, data->idx * sizeof(customer));
    free(temp_data);
}

void readCustomers(customerData *data, struct CustomerFilter *filters, int filterSize)
{
    int index = data->idx;

    for (int i = 0; i < index; i++) {
        customer *current_customer = &data->data[i];
        int filtered = filterCustomer(*current_customer, filters, filterSize);

        if (filtered) {
            printf("%-3d ", i);
            printf("Name: %-20s ", current_customer->name);
            printf("Email: %-40s", current_customer->email);
            printf("Phone: %s\n", current_customer->phone);
        }
    }
}

void createCustomer(customerData *data, char *proto_id, char name[], char email[], char phone[])
{
    char *id = proto_id;

    if (id == NULL) {
        id = idGenerator(proto_id, 20);
    }

    data->idx++;
    data->data = (customer *)realloc(data->data, data->idx * sizeof(customer));

    customer *new_customer = &data->data[data->idx - 1];
    new_customer->id = strdup(id);
    new_customer->name = strdup(name);
    new_customer->email = strdup(email);
    new_customer->phone = strdup(phone);
}

void saveCustomers(const char *filename, customerData *data)
{
    FILE *file = fopen(filename, "w");

    for (int i = 0; i < data->idx; i++) {
        fprintf(file, "%s;%s;%s;%s\n", data->data[i].id, data->data[i].name, data->data[i].email, data->data[i].phone);
    }

    fclose(file);
}

void loadCustomer(const char *filename, customerData *data)
{
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("No current file to open\n");
        return;
    }

    char line[256];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;

        char *id, *name, *email, *phone;

        id = strtok(line, ";");
        name = strtok(NULL, ";");
        email = strtok(NULL, ";");
        phone = strtok(NULL, ";");

        createCustomer(data, id, name, email, phone);
    }

    fclose(file);
}

void createFilter(char filterOption, struct CustomerFilter **filter, int *filterLength)
{    
    while (tolower(filterOption) == 'y') {
        int c;
        char buffer[100];
        printf("Filter expression (e.g. name == Victor): ");
        while ((c = getchar()) != '\n' && c != EOF);
        fgets(buffer, 100, stdin);

        char *field, *operator, *value;

        field = strtok(buffer, " ");
        operator = strtok(NULL, " ");
        value = strtok(NULL, " ");

        enum CT comp;
        char *keyOptions[] = {"id", "name", "email", "value"};
        char *opOptions[] = {"==", "<>"};
        int keyFound = 0;
        int opFound = 0;

        for (int i = 0; i < sizeof(keyOptions)/sizeof(keyOptions[0]); ++i) {
            if (strcmp(field, keyOptions[i]) == 0) {
                keyFound = 1;
                break;
            }
        }

        for (int i = 0; i < sizeof(opOptions)/sizeof(opOptions[0]); i++) {
            if (strcmp(operator, opOptions[i]) == 0) {
                opFound = 1;
                break;
            }
        }

        if (!keyFound) {
            printf("\nInvalid key at filter expression\n");
            return;
        }

        if (!opFound) {
            printf("\nInvalid operator at filter expression\n");
            return;
        }

        (*filterLength)++;

        struct CustomerFilter *ptr = *filter;
        ptr = malloc(*filterLength * sizeof(struct CustomerFilter));
        ptr[*filterLength-1].key = malloc(strlen(field)+1);
        ptr[*filterLength-1].comp = (strcmp(operator, "==") == 0) ? EQUAL : NOT_EQUAL;
        ptr[*filterLength-1].value = malloc(strlen(value)+1);
        ptr[*filterLength-1].closureLen = 0;

        field[strlen(field)] = '\0';
        value[strlen(value)-1] = '\0';

        strcpy(ptr[*filterLength-1].key, field);
        strcpy(ptr[*filterLength-1].value, value);

        *filter = ptr;

        printf("\nEnter another filter expression? (y/N): ");
        scanf("%c", &filterOption);
        printf("\n");
    }
}

void getUserInput(char *prompt, char *buffer) {
    printf("\n%s: ", prompt);
    scanf("%s", buffer);
}

void handleCreate(customerData *customers) {
    char *fields[3];
    char *fieldsNames[] = {"Name", "Email", "Phone"};
    char buffer[100];

    for (int i = 0; i < 3; i++) {
        getUserInput(fieldsNames[i], buffer);
        fields[i] = strdup(buffer);
    }

    createCustomer(customers, NULL, fields[0], fields[1], fields[2]);
}

void handleRead(customerData *customers) {
    char filterOption;
    struct CustomerFilter *filters = NULL;
    int filterLength = 0;

    printf("\nWould you like to filter the results? (y/N): ");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    scanf("%c", &filterOption);

    if (tolower(filterOption) == 'y') {
        createFilter(filterOption, &filters, &filterLength);
    }

    readCustomers(customers, filters, filterLength);
}

void handleUpdate(customerData *customers) {
    char upFilterOption;
    struct CustomerFilter *filters = NULL;
    int filterLength = 0;

    createFilter('y', &filters, &filterLength);
    
    struct UpdateStatement statement;
    customer new_customer;

    statement.id = 0;
    statement.name = 0;
    statement.email = 0;
    statement.phone = 0;

    char *fields[4];
    char *fieldsNames[] = {"Id", "Name", "Email", "Phone"};
    char buffer[100];

    for (int i = 0; i < 4; i++) {
        printf("\n%s: ", fieldsNames[i]);
        fgets(buffer, 100, stdin);
        
        if (strspn(buffer, " \t\n") != strlen(buffer)) {
            buffer[strcspn(buffer, "\n")] = '\0';
            fields[i] = malloc(strlen(buffer) + 1);
            strcpy(fields[i], buffer);
        } else {
            fields[i] = "";
        }
    }

    if (strlen(fields[0]) > 0) {
        statement.id = 1;
        new_customer.id = malloc(strlen(fields[0]));
        strcpy(new_customer.id, fields[0]);
    } 
    if (strlen(fields[1]) > 0) {
        statement.name = 1;
        new_customer.name = malloc(strlen(fields[1]));
        strcpy(new_customer.name, fields[1]);
    } 
    if (strlen(fields[2]) > 0) {
        statement.email = 1;
        new_customer.email = malloc(strlen(fields[2]));
        strcpy(new_customer.email, fields[2]);
    } 
    if (strlen(fields[3]) > 0) {
        statement.phone = 1;
        new_customer.phone = malloc(strlen(fields[3]));
        strcpy(new_customer.phone, fields[3]);
    }

    updateCustomer(customers, new_customer, statement, filters, filterLength);
}

void handleDelete(customerData *customers) {
    char upFilterOption;
    struct CustomerFilter *filters = NULL;
    int filterLength = 0;

    createFilter('y', &filters, &filterLength);
    deleteCustomer(customers, filters, filterLength);
}

int main(void) {
    const char filename[] = "customers";
    customerData customers;
    customers.idx = 0;
    customers.data = NULL;
    loadCustomer(filename, &customers);

    int option = 0;

    while (option < 5) {
        printf("\n");
        printf("Choose an option to do: \n");
        printf("\n[1] Create a new customer in database");
        printf("\n[2] Read customers from the database");
        printf("\n[3] Update customers data");
        printf("\n[4] Delete a customer");
        printf("\n[5] Quit and save program state");
        printf("\n[6] Quit without saving the program state\n");
        printf("\nSelected option: ");
        
        scanf("%d", &option);
        printf("\n");

        switch (option) {
            case 1:
                handleCreate(&customers);
                break;
            
            case 2:
                handleRead(&customers);
                break;
            
            case 3:
                handleUpdate(&customers);
                break;

            case 4:
                handleDelete(&customers);
                break;

            case 5:
                saveCustomers(filename, &customers);
                break;
                return 0;

            case 6:
                break;
                return 0;

            default:
                break;
        }
    }

    return 0;
}