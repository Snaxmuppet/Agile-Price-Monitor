const TimeNow = new Date();

// Get the start time of the current period 
var currentPeriodStart = global.get("currentPeriodStart");

// Get todays prices
const todaysAgilePrices = global.get("todaysAgilePeriods");

// Filter the array to contain only those periods that include the current period and after>

var futureAgilePeriods = todaysAgilePrices.results.filter((el) => el.valid_from >= currentPeriodStart);

msg.futureAgilePeriods = futureAgilePeriods;

console.log("Start new run...");
console.log(futureAgilePeriods);

// -------------------------------------------------------------------------
// We now have an array of all periods forward from now

// So now calculate the best periods

// -------------------------------------------------------------------------

var n = futureAgilePeriods.length;
var min_sum = 0;
var max_sum = 0;
var temp = 0;
var periodStartTime = "";
var periodStartPrice = 0;
var periodAvgMinPrices=[];
var periodSumMinPrices = [];
var periodStartTimes = [];
var periodStartPrices = [];

if (n == 0) 
   return msg;
// let h = 8;
for (let h = 1;h <= 12; h++) {
    min_sum_of_subarray(futureAgilePeriods, n, h);
   // bubbleSort(periodAvgMinPrices, periodStartTimes, periodAvgMinPrices.length);
    periodAvgMinPrices[h] = min_sum / h;
    //periodStartTimes[h] = periodStartTime;
    //periodStartPrices[h] = periodStartPrice;
}

msg.periodAvgMinPrices = periodAvgMinPrices;
msg.periodStartTimes = periodStartTimes;
msg.periodStartPrices = periodStartPrices;

return msg;

// -------------------------------------------------------------------------

function min_sum_of_subarray(arr, n, k) {
    min_sum = 99999;
    for (let i = 0; i + k <= n; i++) {
        let temp = 0;
        for (let j = i; j < i + k; j++) {
            temp += arr[j].value_inc_vat;
        }
        if (temp < min_sum)
            min_sum = temp;
    }

    return;
}
// Sort
function bubbleSort(arr, arr2, n) {
    var i, j, temp, temp2;
    var swapped;
    for (i = 0; i < n - 1; i++) {
        swapped = false;
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                // Swap arr[j] and arr[j+1]
                temp = arr[j];
                temp2 = arr2[j];

                arr[j] = arr[j + 1];
                arr[j + 1] = temp;

                arr2[j] = arr2[j + 1];
                arr2[j + 1] = temp2;

                swapped = true;
            }
        }

        // IF no two elements were 
        // swapped by inner loop, then break
        if (swapped == false)
            break;
    }
}

// Round
function roundTo(n, place) {
    return +(Math.round(n + "e+" + place) + "e-" + place);
}