// Define a function that takes an array of objects with startDate, endDate and cost properties
function calculateAverageCost(array) {
    // Initialize an empty object to store the results
    let result = {};
  
    // Loop through the array
    for (let item of array) {
      // Get the start date and time of the item
      let startDate = new Date(item.startDate);
  
      // Get the end date and time of the item
      let endDate = new Date(item.endDate);
  
      // Get the cost of the item
      let cost = item.cost;
  
      // Calculate the duration of the item in minutes
      let duration = (endDate - startDate) / (60 * 1000);
  
      // Calculate the cost per minute of the item
      let costPerMinute = cost / duration;
  
      // Round down the start date and time to the nearest two-hour interval
      let roundedStartDate = new Date(
        Math.floor(startDate.getTime() / (2 * 60 * 60 * 1000)) * (2 * 60 * 60 * 1000)
      );
  
      // Round up the end date and time to the nearest two-hour interval
      let roundedEndDate = new Date(
        Math.ceil(endDate.getTime() / (2 * 60 * 60 * 1000)) * (2 * 60 * 60 * 1000)
      );
  
      // Loop from the rounded start date and time to the rounded end date and time with a two-hour increment
      for (
        let date = roundedStartDate;
        date < roundedEndDate;
        date = new Date(date.getTime() + 2 * 60 * 60 * 1000)
      ) {
        // Convert the date and time to a string
        let dateString = date.toLocaleString();
  
        // Calculate the overlap minutes between the item and the two-hour period
        let overlapMinutes = Math.min(endDate, date) - Math.max(startDate, date);
  
        // Calculate the overlap cost by multiplying the cost per minute and the overlap minutes
        let overlapCost = costPerMinute * overlapMinutes;
  
        // If the result object already has a key for the date and time, update the sum and count values
        if (result[dateString]) {
          result[dateString].sum += overlapCost;
          result[dateString].count += overlapMinutes;
        }
        // Otherwise, create a new key with the initial sum and count values
        else {
          result[dateString] = { sum: overlapCost, count: overlapMinutes };
        }
      }
    }
  
    // Loop through the result object
    for (let key in result) {
      // Calculate the average cost for each two-hour period by dividing the sum by the count
      let averageCost = result[key].sum / result[key].count;
  
      // Format the average cost to two decimal places
      let averageCostString = averageCost.toFixed(2);
  
      // Display the result
      console.log(
        "The average cost for the two-hour period starting from " +
          key +
          " is: " +
          averageCostString
      );
    }
  }
  
  // Example array
  let array = [
    { startDate: "2023-10-31T16:18:29.000Z", endDate: "2023-10-31T16:48:29.000Z", cost: 12.5 },
    { startDate: "2023-10-31T17:45:13.000Z", endDate: "2023-10-31T18:15:13.000Z", cost: 15.3 },
    { startDate: "2023-10-31T18:22:47.000Z", endDate: "2023-10-31T18:52:47.000Z", cost: 9.8 },
  