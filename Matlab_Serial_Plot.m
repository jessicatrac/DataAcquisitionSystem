%;*********************************************************************                  
%;*                       McMaster University                         *
%;*                      2DP4 Microcontrollers                        *
%;*                          Lab Section 05                           * 
%;*                  Jessica Trac Tracj2 1404451                      *
%;*********************************************************************
%;*********************************************************************
%;*                          Final Project                            *
%;*         ADC Conversion of analog signal from transducer,          *
%;*    which is communicated to PC through serial communication.      *
%;*********************************************************************

delete(instrfindall); % delete all port objects
data = zeros(1,100); % hold 100 samples per graph
 
% Set up serial communication link
s = serial('COM4'); % serial port
s.BaudRate = 19200; % serial baud rate 19200 bps
s.Terminator = 'CR'; 
fopen(s); % open object

% data acquisition settings
resol = 8; % bits
voltage = 5; % [V]
stepsize = 5/((2^resol)-1); 

while(true)
    i = 1;
     while i <= 100;
         data(1,i) = str2double(fgetl(s)); % read new line from file, converted to a double
         i = i + 1;
     end

    data = stepsize*data; % convert digital value to voltage level
    plot(data);
    title('Jessica Trac 001404451 tracj2');
    ylabel('Voltage (V)'); xlabel('Time'); % 'Time' = sample number
    ylim([0 5 ]); % scale from 0V - 5V 
    pause(0.05);
end