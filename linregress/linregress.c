#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_cdf.h>

typedef struct baseTable
{
    int size;
    int *idx;
    int *x;
    int *y;
    int *x2;
    int *y2;
    int *xy;
    int x_sum;
    int y_sum;
    int x2_sum;
    int y2_sum;
    int xy_sum;

}
baseTable;

typedef struct linearRegression
{
    double b0;
    double b1;
    double lower_b0;
    double upper_b0;
    double lower_b1;
    double upper_b1;
    double SSR;
    double SSX;
    double SST;
    double SE;
    double SE_b1;
    double SE_b0;
    double p_value;
    double r_value;
}
linearRegression;

int *range(int start, int stop, int step)
{
    int length = stop - start + 1;
    int *array = malloc(length * sizeof(int));

    for (int i = start, c = 0; i < length; i+=step, c++) {
        array[c] = i;
    }

    return array;
}

int getArrSize(int *arr)
{
    int size = 0;
    while (arr[size] != '\0') {
        size++;
    }
    return size;
}

int *mapProd(int *arr1, int *arr2, int size)
{
    int *map = malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        map[i] = arr1[i] * arr2[i];
    }

    return map;
}

int *mapPow(int *arr, int exp, int size)
{
    int *map = malloc(size * sizeof(int));

    for (int i = 0; i < size; i++) {
        map[i] = pow(arr[i], exp);
    }

    return map;
}

int sum(int *arr, int size)
{
    int sum = 0;

    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }

    return sum;
}

const baseTable *createTable(int *x, int *y)
{
    baseTable *table = malloc(sizeof(baseTable));

    int size_x = getArrSize(x);
    int size_y = getArrSize(y);

    if (size_x != size_y) return NULL;

    int size = size_x;

    table->size = size;
    table->idx = range(0, size, 1);

    table->x = x;
    table->y = y;
    table->x2 = mapPow(x, 2, size);
    table->y2 = mapPow(y, 2, size);
    table->xy = mapProd(x, y, size);

    table->x_sum = sum(x, size);
    table->y_sum = sum(y, size);
    table->x2_sum = sum(table->x2, size);
    table->y2_sum = sum(table->y2, size);
    table->xy_sum = sum(table->xy, size);

    return table;
}

double calculatePValue(double SSR, double SSX, double b1, int df, double SE_b1)
{
    double t_statistic = b1 / SE_b1;

    double p_value_one_sided = gsl_cdf_tdist_P(fabs(t_statistic), df);
    double p_value_two_sided = 2.0 * (1.0 - p_value_one_sided);

    return p_value_two_sided;
}

double calculateRValue(baseTable t) {
    double numerator = (double)(t.size * t.xy_sum) - (double)(t.x_sum * t.y_sum);
    double denominator_x = sqrt((double)(t.size * t.x2_sum) - pow((double)t.x_sum, 2));
    double denominator_y = sqrt((double)(t.size * t.y2_sum) - pow((double)t.y_sum, 2));
    double r_value = numerator / (denominator_x * denominator_y);

    return r_value;
}

linearRegression simplelinRegress(baseTable t, double alpha)
{
    linearRegression linregress;

    double x_mean = (double)t.x_sum / (double)t.size;
    double y_mean = (double)t.y_sum / (double)t.size;

    double numerator = 0;
    double denominator = 0;

    for(int i = 0; i < t.size; i++) {
        numerator += (t.x[i] - x_mean) * (t.y[i] - y_mean);
        denominator += pow(t.x[i] - x_mean, 2);
    }

    double b1 = numerator / denominator;
    double b0 = y_mean - (b1 * x_mean);

    linregress.b0 = b0;
    linregress.b1 = b1;

    double SSR = 0.0;
    double SSX = 0.0;
    double SST = 0.0;

    for (int i = 0; i < t.size; i++) {
        double y_pred = b0 + b1 * t.x[i];
        double residual = t.y[i] - y_pred;
        SSR += pow(residual, 2);
    }

    for (int i =0; i < t.size; i++) {
        SSX += pow(t.x[i] - x_mean, 2);
    }

    for (int i = 0; i < t.size; i++) {
        SST += pow(t.y[i] - y_mean, 2);
    }

    linregress.SSR = SSR;
    linregress.SSX = SSX;
    linregress.SST = SST;

    double SE = sqrt(SSR / (t.size - 2));
    double SE_b1 = sqrt(pow(SE, 2)/SSX);
    double SE_b0 = SE_b1 * sqrt((double)sum(t.x2, t.size) / t.size);

    linregress.SE = SE;
    linregress.SE_b1 = SE_b1;
    linregress.SE_b0 = SE_b0;

    int df = t.size-2;
    double p_value = calculatePValue(SSR, SSX, b1, df, SE_b1);
    double r_value = calculateRValue(t);
    double t_critical = gsl_cdf_tdist_Pinv(1.0 - alpha / 2.0, df);

    linregress.p_value = p_value;
    linregress.r_value = r_value;

    linregress.lower_b1 = linregress.b1 - t_critical * SE_b1;
    linregress.upper_b1 = linregress.b1 + t_critical * SE_b1;
    linregress.lower_b0 = linregress.b0 - t_critical * SE_b0;
    linregress.upper_b0 = linregress.b0 + t_critical * SE_b0;

    return linregress;
}

double predictYValue(linearRegression lin_reg, double x_value) {
    return lin_reg.b0 + lin_reg.b1 * x_value;
}

double predictXValue(linearRegression lin_reg, double y_value) {
    return (y_value - lin_reg.b0) / lin_reg.b1;
}


int main(void)
{
    int x[5] = {1, 2, 3, 4, 5};
    int y[5] = {1, 1, 2, 2, 4};

    const baseTable *table = createTable(x, y);

    if (!table) {
        printf("The axis must have the same size");
        return 1;
    }

    double alpha = 0.05;
    linearRegression linregress = simplelinRegress(*table, alpha);
    
    printf("Intercep: %f\n", linregress.b0);
    printf("Slope: %f\n", linregress.b1);

    return 0;
}