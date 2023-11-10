const TimeNow = new Date();

// Get the start time of the current period 
var currentPeriodStart = global.get("currentPeriodStart");

// Get todays prices
const todaysAgilePrices = global.get("todaysAgilePeriods");

// Filter the array to contain only those periods that include the current period and after>

var futureAgilePeriods = todaysAgilePrices.results.filter((el) => el.valid_from > currentPeriodStart);

// periods are in reverse sequence so sort into ascending date/time seq
var n = futureAgilePeriods.length;
bubbleSort(futureAgilePeriods, n);

msg.futureAgilePeriods = futureAgilePeriods;

console.log("Start new run...");
console.log(futureAgilePeriods);

// -------------------------------------------------------------------------
// We now have an array of all periods forward from now

// So now calculate the best periods

// -------------------------------------------------------------------------


var min_sum = 0;
var max_sum = 0;
var periodStartTime = "";
var periodStartPrice = 0;
var periodAvgMinPrices = [];
var periodSumMinPrices = [];
var periodStartTimes = [];
var periodStartPrices = [];

if (n == 0)
    return msg;

for (let h = 1; h <= 12; h++) {

    min_sum_of_subarray(futureAgilePeriods, n, h);

    periodAvgMinPrices[h] = roundTo(min_sum / h, 2);
    periodStartTimes[h] = periodStartTime;

}

msg.periodAvgMinPrices = periodAvgMinPrices;
msg.periodStartTimes = periodStartTimes;

return msg;

// -------------------------------------------------------------------------

function min_sum_of_subarray(arr, n, k) {
    min_sum = 99999;
    for (let i = 0; i + k <= n; i++) {
        let temp = 0;
        for (let j = i; j < i + k; j++) {
            temp += arr[j].value_inc_vat;
        }
        if (temp < min_sum) {
            min_sum = temp;
            periodStartTime = arr[i].valid_from;

            console.log("save start time");
            console.log(periodStartTime);
        }

    }

    return;
}

// Round
function roundTo(n, place) {
    return +(Math.round(n + "e+" + place) + "e-" + place);
}

// Sort
function bubbleSort(arr, n) {
    var i, j, temp;
    var swapped;
    for (i = 0; i < n - 1; i++) {
        swapped = false;
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j].valid_from > arr[j + 1].valid_from) {

                // Swap arr[j] and arr[j+1]
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = true;
            }
        }

        // IF no two elements were 
        // swapped by inner loop, then break
        if (swapped == false)
            break;
    }
}
