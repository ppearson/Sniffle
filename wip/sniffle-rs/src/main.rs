/*
 Sniffle
 Copyright 2024 Peter Pearson.

 Licensed under the Apache License, Version 2.0 (the "License");
 You may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ---------
*/

use std::env;
use std::io::{BufReader, BufRead};

const K_CUMULATIVE_DAYS_IN_YEAR_FOR_MONTH: [u32; 12] =          [ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 ];
const K_CUMULATIVE_DAYS_IN_YEAR_FOR_MONTH_LEAPYEAR: [u32; 12] = [ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 ];

// Minimum sub-set of Sniffle (C++) functionality, just replicating the timestamp delta functionality for the moment...

fn main() {
    let args: Vec<String> = env::args().collect();
    
    if args.len() < 2 {
        eprintln!("Error: Single file not specified on command line.");
        return;
    }

    let mut list_of_files_path: Option<String> = None;
    if args.len() == 3 {
        if args[1] == "-l" || args[1] == "--list" {
            list_of_files_path = Some(args[2].clone());
        }
    }


    let gap_threshold: u64 = 10 * 60;

    if let Some(list_files_path) = list_of_files_path {
        eprintln!("Processing list of input files...");
        process_input_list(&list_files_path, gap_threshold);
    }
    else {
        eprintln!("Processing input file...");
        check_file(&args[1], gap_threshold);
    }

    eprintln!("\nCompleted.")
}

fn process_input_list(list_path: &str, max_gap_seconds: u64) {
    let file = std::fs::File::open(list_path);
    if file.is_err() {
        eprintln!("Error: couldn't open file: '{}'", list_path);
        return;
    }

    let reader = BufReader::new(file.unwrap());

    for line in reader.lines() {
        let line = line.unwrap();

        // ignore empty lines and comments
        if line.is_empty() {
            continue;
        }

        check_file(&line, max_gap_seconds);
    }
}

fn check_file(path: &str, max_gap_seconds: u64) {
    let file = std::fs::File::open(path);
    if file.is_err() {
        eprintln!("Error: couldn't open file: '{}'", path);
        return;
    }

    let mut last_line = String::new();
    let mut last_timeval: u64 = 0;

    let mut cumulative_days_in_year_for_month: Option<&[u32; 12]> = None; 

    let mut found_count = 0;

    let reader = BufReader::new(file.unwrap());

    for line in reader.lines() {
        let line = line.unwrap();

        // ignore empty lines and comments
        if line.is_empty() {
            continue;
        }

        if !line.starts_with('[') {
            continue;
        }

        if line.len() < 22 {
            continue;
        }

        let ts_end_pos = line.find(']');
        if ts_end_pos.is_none() {
            continue;
        }

        let ts_end_pos = ts_end_pos.unwrap();

        let extracted_ts = &line[1..ts_end_pos];

        if extracted_ts.len() < 19 {
            continue;
        }

        // for the moment, copy what the C++ version did in terms of extracting digits manually,
        // as at least in c++ doing thi was much more efficient CPU-time-wise than using sscanf() or strptime(),
        // however I haven't actually measured whether it's as much of a problem in Rust
        let mut year_val = get_number_digit_val_from_string(extracted_ts, 0).unwrap_or(0) * 1000;
        year_val += get_number_digit_val_from_string(extracted_ts, 1).unwrap_or(0) * 100;
        year_val += get_number_digit_val_from_string(extracted_ts, 2).unwrap_or(0) * 10;
        year_val += get_number_digit_val_from_string(extracted_ts, 3).unwrap_or(0);

        if cumulative_days_in_year_for_month.is_none() {
            
            // exactly divisible by 400, not exactly devisible by 100
            let is_leap_year = ((year_val % 400 == 0) || (year_val % 100 != 0)) && (year_val % 4 == 0);
            if is_leap_year {
                cumulative_days_in_year_for_month = Some(&K_CUMULATIVE_DAYS_IN_YEAR_FOR_MONTH_LEAPYEAR);
            }
            else {
                cumulative_days_in_year_for_month = Some(&K_CUMULATIVE_DAYS_IN_YEAR_FOR_MONTH);
            }
        }

        let mut month_val = get_number_digit_val_from_string(extracted_ts, 5).unwrap_or(0) * 10;
        month_val += get_number_digit_val_from_string(extracted_ts, 6).unwrap_or(0);

        let mut day_val = get_number_digit_val_from_string(extracted_ts, 8).unwrap_or(0) * 10;
        day_val += get_number_digit_val_from_string(extracted_ts, 9).unwrap_or(0);

        let mut hour_val = get_number_digit_val_from_string(extracted_ts, 11).unwrap_or(0) * 10;
        hour_val += get_number_digit_val_from_string(extracted_ts, 12).unwrap_or(0);

        let mut minute_val = get_number_digit_val_from_string(extracted_ts, 14).unwrap_or(0) * 10;
        minute_val += get_number_digit_val_from_string(extracted_ts, 15).unwrap_or(0);

        let mut second_val = get_number_digit_val_from_string(extracted_ts, 17).unwrap_or(0) * 10;
        second_val += get_number_digit_val_from_string(extracted_ts, 18).unwrap_or(0);

        let num_days_since_start_of_year_to_month = (cumulative_days_in_year_for_month.unwrap()[(month_val - 1) as usize]) as u64;

        let mut current_time: u64 = (year_val as u64 * 365 * 31 * 24 * 60 * 60) + (num_days_since_start_of_year_to_month * 24 * 60 * 60);
        current_time += (day_val as u64 * 24 * 60 * 60) + (hour_val as u64 * 60 * 60) + (minute_val as u64 * 60) + second_val as u64;

        if last_timeval != 0 {
            let time_delta = current_time - last_timeval;

            if time_delta >= max_gap_seconds {
                if found_count > 0 {
                    // print a blank line to separate out the new timestamp pair
                    println!();
                }
                else {
                    // print name of file after a newline
                    println!("\n{} :\n", path);
                }

                println!("{}\n{}", last_line, line);

                found_count += 1;
            }
        }

        last_timeval = current_time;

        last_line = line.clone();
    }
}

fn get_number_digit_val_from_string(string_val: &str, digit_pos: usize) -> Option<u32> {
    if let Some(char_at_pos) = string_val.chars().nth(digit_pos) {
        let diff = (char_at_pos as u8) - ('0' as u8);
        return Some(diff.into());
    } 

    None
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_1() {
        assert_eq!(get_number_digit_val_from_string("2024-07-10 11:32:55", 0), Some(2));
        assert_eq!(get_number_digit_val_from_string("2024-07-10 11:32:55", 1), Some(0));
        assert_eq!(get_number_digit_val_from_string("2024-07-10 11:32:55", 2), Some(2));
        assert_eq!(get_number_digit_val_from_string("2024-07-10 11:32:55", 3), Some(4));
        assert_eq!(get_number_digit_val_from_string("2024-07-10 11:32:55", 5), Some(0));
        assert_eq!(get_number_digit_val_from_string("2024-07-10 11:32:55", 6), Some(7));
       

    }
}