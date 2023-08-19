import { Component, OnInit } from '@angular/core';
import { Router } from '@angular/router';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})
export class AppComponent implements OnInit {
  title = 'onprem';

  constructor(private rt: Router) {}

  ngOnInit(): void {
    this.rt.navigateByUrl('/login');
  }
}
