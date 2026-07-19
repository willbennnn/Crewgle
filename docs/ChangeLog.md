# Changelog
Below is a list of features associated with past, current, and future iterations of the crewgle

## VERSION 0.1
0.1 was the very basics of getting a working web app; basic UI, working backend API, basic SQL tables, etc.  

### Version 0.1.1
- [x] Database linked api containing services and methods for:
  - [x] Users (Athletes, Coaches, Coxswains, Captains)
  - [x] Practices
  - [x] Lineups
  - [x] Attendance
  - [x] Availability
- [x] Dummy frontend for database seed data visualization
- [x] Coaches can create/edit practices
- [x] Coaches can create/edit lineups
- [x] Coaches, Officers, Captains can take attendance for specific practices
- [x] Everyone can edit their availability 
- [x] Coaches can input substitutes into boats

### Version 0.1.2
- [x] UI cleanup
- [x] Erg workout submission page which includes:
  - [x] Name of the workout
  - [x] Total meters rowed
  - [x] Time rowed
  - [x] Type of workout (UT3/2/1, AT, TR, AC/VO2, AP)
  - [x] Is it a test piece? (pre-inputed options such as 2k, 5k, 30r20, 6k, etc.)
  - [x] /500m split
  - [x] Day submitted
- [x] Week view page where users can see practices & lineups for the week

## VERSION 0.2
0.2 was where the first real rendition of the webapp was developed, with major UX improvments (i.e. completely changing the client-side layout), new features, and bug fixes. Change logs are more detailed from here onwards because I actually started writing down what I wanted to accomplish.

### Version 0.2.1
- [x] Make the dashboard smaller, allow the page to take up the majority of the screen
- [x] Separate the entire interface into two types of views:
  - [x] **“MY” VIEW:** which includes a page to set your availability, a page where you can view only the lineups you’ve been assigned to (“my practices”), a page that includes your attendance history, and a page that includes your workout history and any incomplete makeup pieces you were assigned
  - [x] **“TEAM” VIEW:**  which includes a page where you can see all lineups at each practice for the week, a training page that includes leaderboards for “most meters done” for the overall team/by squad and a leaderboard for certain benchmark pieces (2k,5k,etc.) by squad, the rosters page (more on that below) and the regattas page (ill make that look nicer in version 4)  *(note only coaches will be able to see everyone’s availability. All other roles will only be able to see/edit their own)**
- [x] Create a separate interface within the app for coaches only for practice & lineup creation:
  - [x] Allows Coaches to see all lineups for all practices for a single week (the week being viewed is selectable)
  - [x] Can click on a part of a day not containing a practice card to create a new practice on that day
  - [x] Also another button (maybe it says “bulk add practices”) where the creation of the practice is the same, but allows you to create as many copies of that practice as you want by specifying which days to copy to on a separate field
  - [x] Practice cards should also have a small option to copy to other days when clicked on
  - [x] Shows a tally of the amount of land and water practices scheduled for each athlete that week on a side bar that dynamically updates
  - [x] Click on individual day cards which brings up a zoomed view of just that day, where:
  - [x] Allows for filtered searching of athletes by availability status, squad, number of practices scheduled for that week, and rowing side (if a sweep boat) on a separate pane
  - [x] The above search results are each tiles that can be dragged and dropped into empty seats (or swapped if the seat is filled)
  - [x] On another separate pane, have tiles that have shells with blank spots inside them. Once a shell is drag and dropped into a respective practice, drag the names from the aforementioned pane into the shell.
- [x] “Invite to register” where an email can be entered for the platform to send a request to register to

### Version 0.2.2
- [x] Add permissions so only certain roles can perform certain tasks
- [x] Add an auth page for login, registration
- [x] Fix some visual bugs like columns displaying incorrectly on the rosters page, availability button states not updating, and more
- [x] Change the seed data to be names from TAMU, not randomly generated

### Version 0.2.3
- [x] Add a "Forgot Password?" option on the login page, only a dev link for now, an email API will be added in the future

## VERSION 0.3
Another major new interface for equipment mangaging was added, along with major overhauls to the erg workout and week view interfaces. Additional features such as lineup grouping in the practice builder and some "to-do"s from previous versions were also knocked out

### Version 0.3.1
- [ ] **Equipment Management Interface**
    - [ ] **FOR BOATS** ~ Enter boats name, year, make, manufacturer, class (sweep/scull/both), coxxed/uncoxxed, size, and notes (for specific rigging information)
    - [ ] **FOR OARS** ~ Enter Oar manufacturer, make, squad, number (I think we should start numbering oars), inboard, and notes (for paint condition, damage, etc.)
    - [ ] **FOR ERGS** ~ Manufacturer, model, number, last service date, lifetime meters at last service, notes
    - [ ] **FOR “OTHER”** (random stuff like slides, tents, launches, etc) ~ Name, Manufacturer, Model, Year, Number, Serial Number (all fields optional)
    - [ ] All equipment should have the option to be flagged as “in need of maintenance” (yellow status) or broken (red status) by coaches, captains, officers by default***
    - [ ] When equipment is flagged, a ticket is created that can be resolved by officers and coaches

### Version 0.3.2
- [ ] Change week view to calendar with modes for month view, week view, and day view (like Google Calendar)
    - [ ] Add cross platform calendar using APIs that can link events to google/apple calendar
          
- [ ] Make the erg workout section into a more full interface (something like strava, but with more of a rowing focus) *(specify more later)*

### Version 0.3.3
- [ ] Add an email API to the password reset system
    
- [ ] **OPTIONAL LINEUP GROUPING** ~ instead of having to make lineups for practices manually for every practice, have the option to create a lineup group   (Ex: Men’s 1V8) that can be applied easier

## VERSION 0.4
The first user tested version, user testing began on version 0.4.2. Major additions include the regatta tab using the regatta central API (https://api.regattacentral.com/v4/apiV4.jsp), ... (More later)


