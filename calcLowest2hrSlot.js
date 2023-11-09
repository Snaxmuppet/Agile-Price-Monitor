function max_sum_of_subarray(arr, n, k)
{
	let max_sum = 0;
	for (let i = 0; i + k <= n; i++) {
		let temp = 0;
		for (let j = i; j < i + k; j++) {
			temp += arr[j];
		}
		if (temp > max_sum)
			max_sum = temp;
	}

	return max_sum;
}

let arr = [ 1, 4, 2, 10, 2, 3, 1, 0, 20 ];
let k = 4;
let n = arr.length;
let max_sum;

// by brute force
max_sum = max_sum_of_subarray(arr, n, k);
console.log(max_sum);

// This code is contributed by ritaagarwal.
